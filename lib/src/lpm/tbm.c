#include "tbm.h"
#include <stdio.h>
/**
 * @brief Count all ones up to given position
 * @param key
 * @param bit_position stop marker
 * @return number of ones in bitmap
 */
uint16_t _tbm_bitsum(uint32_t * bitmap, uint16_t bit_position)
{
	assert(bitmap != NULL);

	uint16_t sum = 0;
	uint32_t map;
	uint8_t num_bits;

	for(int i = 0; i * 32 < bit_position; ++i)
	{
		num_bits = 32;
		// cut off the bits at bit position and after that
		if((i + 1) * 32 > bit_position) num_bits = (bit_position - i*32);
		map = GET_BITS_LSB(bitmap[i], num_bits);
		sum += __builtin_popcount(map);

	}

	return sum;
}

/**
 * @brief Set zeros to bitmap
 * @param bitmap
 * @param size number of bytes
 */
void _tbm_zeros(uint32_t * bitmap, uint16_t size)
{
	assert(bitmap != NULL);

	memset(bitmap, 0, sizeof(*bitmap) * size);
}

/**
 * @brief Calculate index for rule based on length of match, used only for lookup
 * @param bitmap internal bitmap
 * @param bit_value part of ip address
 * @return index to node->rule
 */
int32_t _tbm_internal_index(uint32_t * bitmap, uint16_t bit_value)
{
	assert(bitmap != NULL);

	int8_t length = STRIDE;
	uint16_t bit_position;

	// from longest match to shortest: NNN, NN*, N*, *
	do
	{
		bit_position = (1 << length) - 1 + bit_value;
		bit_value >>= 1;
	}
	while(length-- >= 0 && GET_BIT_LSB(bitmap[bit_position / 32], bit_position % 32) == 0);

	if(length < 0 && GET_BIT_LSB(bitmap[0], 0) == 0)
	{
		// no matching shorter prefix found, do not return 0 from bitsum
		return -1;
	}

	return _tbm_bitsum(bitmap, bit_position);
}

/**
 * @brief Get number consiting of <length> bits from <position> in <prefix>
 * @param prefix
 * @param position starting position
 * @param length number of bits
 * @return number of <length> bits
 */
uint8_t _get_bits(uint32_t * prefix, uint8_t position, uint8_t length)
{
	uint32_t data;

	if(position / 32 != (position + length - 1) / 32)
	{
		uint8_t index = position/32;
		uint8_t
		data = prefix[index + 1] >> (31 - (position+length) % 32);
		data |= prefix[index] << (length - (position + length) % 32);
		data &= ~(~0 << length);
		return data;
	}
	else
	{
		data = prefix[position / 32];
		return (data >> (32 - length - position)) & ~(~0 << length);
	}
}

/**
 * @brief Lookup node where prefix is located, used by update/remove
 * @param root
 * @param prefix
 * @param prefix_len
 * @param index [out] int value of prefix part which is used as index to internal/external
 * @return node
 */
_tbm_node * _tbm_lookup_internal(lpm_root * root, uint32_t * prefix, uint8_t prefix_len, uint16_t * index)
{
	assert(root != NULL);
	assert(index != NULL);

	uint16_t bit_value;
	uint8_t length;
	uint8_t position = 0;
	_tbm_node * node = root;

	while(position + STRIDE < prefix_len)
	{
		bit_value = _get_bits(prefix, position, STRIDE);
		position += STRIDE;

		if(node->child == NULL) break;
		node = &(node->child[_tbm_bitsum(node->external, bit_value)]);
	}

	length = prefix_len - position;
	bit_value = _get_bits(prefix, position, length);
	*index = INTERNAL_INDEX(length, bit_value);

	return node;
}

/**
 * @brief Extend array for one more item
 * @param node
 * @param bit_value
 * @param shift_child 1 => change size of node->child otherwise node->rule
 * @fixme leak v reallocu
 */
_Bool _tbm_extend(_tbm_node * node, uint16_t bit_value, _Bool shift_child)
{
	assert(node != NULL);

	uint16_t bitsum;
	uint8_t index_start;
	uint16_t all;
	uint32_t * bitmap;

	if(shift_child)
	{
		bitmap = node->external;
		all = _TBM_ALL_EXTERNAL;
	}
	else
	{
		bitmap = node->internal;
		all = _TBM_ALL_INTERNAL;
	}

	bitsum = _tbm_bitsum(bitmap, all);
	index_start = _tbm_bitsum(bitmap, bit_value);

	if(shift_child)
	{
		_tbm_node * new_child;

		//resize, add space for one more child
		new_child = realloc(node->child, (bitsum + 1) * sizeof(_tbm_node));

		if(new_child == NULL) return errno = FASTNET_OUT_OF_MEMORY, 0;

		node->child = new_child;
		//shift all values behind newly added to the right
		memmove(&(node->child[index_start + 1]), &(node->child[index_start]), (bitsum - index_start) * sizeof(_tbm_node));

		node->child[index_start].child = NULL;
		node->child[index_start].rule = NULL;
		_tbm_zeros(node->child[index_start].external, _TBM_SIZE_EXTERNAL);
		_tbm_zeros(node->child[index_start].internal, _TBM_SIZE_INTERNAL);
	}
	else
	{
		_LPM_RULE * new_rule;
		//resize, add space for one more rule
		new_rule = realloc(node->rule, (bitsum + 1) * sizeof(_LPM_RULE));

		if(new_rule == NULL) return errno = FASTNET_OUT_OF_MEMORY, 0;

		node->rule = new_rule;
		//shift all values behind newly added to the right
		memmove(&(node->rule[index_start + 1]), &(node->rule[index_start]), (bitsum - index_start) * sizeof(_LPM_RULE));
	}

	return 1;
}

/**
 * @brief Reduce array by 1
 * @param node
 * @param bit_value
 * @param shift_child 1 => change size of node->child otherwise node->rule
 */
_Bool _tbm_reduce(_tbm_node * node, uint16_t bit_value, _Bool shift_child)
{
	assert(node != NULL);

	uint16_t bitsum;
	uint8_t index_start;
	uint16_t all;
	uint32_t * bitmap;

	if(shift_child)
	{
		bitmap = node->external;
		all = _TBM_ALL_EXTERNAL;
	}
	else
	{
		bitmap = node->internal;
		all = _TBM_ALL_INTERNAL;
	}

	bitsum = _tbm_bitsum(bitmap, all);
	index_start = _tbm_bitsum(bitmap, bit_value);

	if(shift_child)
	{
		_tbm_node * new_child;

		memmove(&(node->child[index_start + 1]), &(node->child[index_start]), (bitsum - index_start - 1) * sizeof(_tbm_node));
		new_child = realloc(node->child, (bitsum - 1) * sizeof(_tbm_node));

		if(new_child == NULL && bitsum != 1) return errno = FASTNET_OUT_OF_MEMORY, 0;

		node->child = new_child;
	}
	else
	{
		_LPM_RULE * new_rule;

		memmove(&(node->rule[index_start + 1]), &(node->rule[index_start]), (bitsum - index_start - 1) * sizeof(_LPM_RULE));
		new_rule = realloc(node->rule, (bitsum - 1) * sizeof(_LPM_RULE));

		if(new_rule == NULL && bitsum != 1) return errno = FASTNET_OUT_OF_MEMORY, 0;

		node->rule = new_rule;
	}

	return 1;
}

/**
 * @brief Destroy node children and free memory
 * @param node
 */
void _tbm_free(_tbm_node * node)
{
	assert(node != NULL);

	free(node->rule);

	for(int i = 0; i < _tbm_bitsum(node->external, _TBM_ALL_EXTERNAL); ++i)
	{
		_tbm_free(&(node->child[i]));
	}

	free(node->child);
}

/**
 * @brief Inicialize TreeBitmap structure
 * @param default_rule rule for default route
 * @return pointer to data
 */
void * _tbm_init(_LPM_RULE default_rule)
{
	lpm_root * root;
	root = malloc(sizeof(_tbm_node));

	if(root == NULL) return errno = FASTNET_OUT_OF_MEMORY, NULL;

	root->child = NULL;
	root->rule = malloc(sizeof(_LPM_RULE));

	if(root->rule == NULL) return errno = FASTNET_OUT_OF_MEMORY, free(root), NULL;

	root->rule[0] = default_rule;
	_tbm_zeros(root->external, _TBM_SIZE_EXTERNAL);
	_tbm_zeros(root->internal, _TBM_SIZE_INTERNAL);
	SET_BIT_LSB(root->internal[0], 0);

	return root;
}

/**
 * @brief Insert <prefix>/<prefix_len> to tbm structure with coresponding <rule>
 * @param root
 * @param prefix ip address
 * @param prefix_len bits of <prefix>
 * @param rule corresponding rule
 * @return succes/error
 */
_Bool _tbm_add(lpm_root * root, uint32_t * prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	assert(root != NULL);

	uint8_t position = 0;
	uint16_t index;
	uint16_t bit_value;
	uint8_t stride_len;

	_tbm_node * node = root;

	while(position + STRIDE < prefix_len)
	{
		bit_value = _get_bits(prefix, position, STRIDE);
		position += STRIDE;

		if(GET_BIT_LSB(node->external[bit_value / 32], bit_value % 32) == 0)
		{
			if(_tbm_extend(node, bit_value, 1) == 0) return 0;
			SET_BIT_LSB(node->external[bit_value / 32], bit_value % 32);
		}

		node = &(((_tbm_node *) node->child)[_tbm_bitsum(node->external, bit_value)]);
	}

	stride_len = prefix_len - position;
	bit_value = _get_bits(prefix, position, stride_len);
	index = INTERNAL_INDEX(stride_len, bit_value);

	if(_tbm_extend(node, index, 0) == 0) return 0;
	SET_BIT_LSB(node->internal[index / 32], index % 32);
	node->rule[_tbm_bitsum(node->internal, index)] = rule;

	return 1;
}

/**
 * @brief Update <prefix>/<prefix_len> to <rule>
 * @param root
 * @param prefix
 * @param prefix_len
 * @param rule
 * @param ipv6
 */
void _tbm_update(lpm6_root * root, uint32_t * prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	assert(root != NULL);

	uint16_t index;
	_tbm_node * node = _tbm_lookup_internal(root, prefix, prefix_len, &index);

	assert(node != NULL);

	node->rule[_tbm_bitsum(node->internal, index)] = rule;
}

/**
 * @brief Remove <prefix>/<prefix_len>
 * @param root
 * @param prefix
 * @param prefix_len
 * @param ipv6
 */
void _tbm_remove(lpm_root * root, uint32_t * prefix, uint8_t prefix_len)
{
	assert(root != NULL);

	uint16_t index;
	_tbm_node * node = _tbm_lookup_internal(root, prefix, prefix_len, &index);

	assert(node != NULL);

	_tbm_reduce(node, index, 0);
	CLEAR_BIT_LSB(node->internal[index / 32], index % 32);
}

/**
 * @brief Free all data
 * @param root
 */
void _tbm_destroy(_tbm_node * root)
{
	if(root == NULL) return;

	_tbm_free(root);
	free(root);
}

/**
 * @brief Lookup for longest prefix match for <key>
 * @param root pointer to tbm data
 * @param key ip address
 * @return rule of longest match
 */
_LPM_RULE _tbm_lookup(lpm6_root * root, uint32_t * key)
{
	assert(root != NULL);

	_tbm_node * node = root;
	_tbm_node * parent = NULL;
	_tbm_node * longest_match_node = root;
	uint8_t position = 0;
	uint16_t bits;
	uint16_t index;
	uint16_t longest_match_index = 0;
	int32_t tmp_index; // internal index can be -1 if not found

	// while there is longer match
	do
	{
		bits =_get_bits(key, position, STRIDE);
		position += STRIDE;

		if(node->rule != NULL && (tmp_index = _tbm_internal_index(node->internal, bits)) != -1)
		{
			longest_match_index = tmp_index;
			longest_match_node = node;
		}
		// index for child pointer is bitsum from <0, (int) bits)
		index = _tbm_bitsum(node->external, bits);

		// move to child
		parent = node;
		node = &(node->child[index]);
	}
	// longest_match_node is parent from current node (node contains child pointer from node operated in code above)
	while(GET_BIT_LSB(parent->external[bits / 32], bits % 32));

	return longest_match_node->rule[longest_match_index];
}
/**
 * @brief Inicialize data for IPv4
 * @param default_rule rule for default route
 * @return pointer to data structure
 */
lpm_root * lpm_init(_LPM_RULE default_rule)
{
	return _tbm_init(default_rule);
}

/**
 * @brief Inicialize data for IPv6
 * @param default_rule rule for default route
 * @return pointer to data structure
 */
lpm6_root * lpm6_init(_LPM_RULE default_rule)
{
	return _tbm_init(default_rule);
}

/**
 * @brief Insert <prefix>/<prefix_len> to tbm structure with coresponding <rule> for IPv4
 * @param root
 * @param prefix ip address
 * @param prefix_len bits of <prefix>
 * @param rule corresponding rule
 * @return succes/error
 */
_Bool lpm_add(lpm_root * root, struct in_addr * prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	return _tbm_add(root, (uint32_t *) prefix, prefix_len, rule);
}

/**
 * @brief Insert <prefix>/<prefix_len> to tbm structure with coresponding <rule> for IPv6
 * @param root
 * @param prefix ip address
 * @param prefix_len bits of <prefix>
 * @param rule corresponding rule
 * @return succes/error
 */
_Bool lpm6_add(lpm6_root * root, struct in6_addr * prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	return _tbm_add(root, (uint32_t *) prefix, prefix_len, rule);
}

/**
 * @brief Update <prefix>/<prefix_len> to <rule> for IPv4
 * @param root
 * @param prefix
 * @param prefix_len
 * @param rule
 */
void lpm_update(lpm_root * root, struct in_addr * prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	_tbm_update(root, (uint32_t *) prefix, prefix_len, rule);
}

/**
 * @brief Update <prefix>/<prefix_len> to <rule> for IPv6
 * @param root
 * @param prefix
 * @param prefix_len
 * @param rule
 */
void lpm6_update(lpm6_root * root, struct in6_addr * prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	_tbm_update(root, (uint32_t *) prefix, prefix_len, rule);
}

/**
 * @brief Remove <prefix>/<prefix_len> for IPv4
 * @param root
 * @param prefix
 * @param prefix_len
 */
void lpm_remove(lpm_root * root, struct in_addr * prefix, uint8_t prefix_len)
{
	_tbm_remove(root, (uint32_t *) prefix, prefix_len);
}

/**
 * @brief Remove <prefix>/<prefix_len> for IPv6
 * @param root
 * @param prefix
 * @param prefix_len
 */
void lpm6_remove(lpm6_root * root, struct in6_addr * prefix, uint8_t prefix_len)
{
	_tbm_remove(root, (uint32_t *) prefix, prefix_len);
}

/**
 * @brief Free all data for IPv6
 * @param root
 */
void lpm_destroy(lpm_root * root)
{
	_tbm_destroy(root);
}

/**
 * @brief Free all data for IPv4
 * @param root
 */
void lpm6_destroy(lpm6_root * root)
{
	_tbm_destroy(root);
}

/**
 * @brief Lookup for longest prefix match for <key> IPv4
 * @param root pointer to tbm data
 * @param key ip address
 * @return rule of longest match
 */
_LPM_RULE lpm_lookup(lpm_root * root, struct in_addr * key)
{
	return _tbm_lookup(root, (uint32_t * ) key);
}

/**
 * @brief Lookup for longest prefix match for <key> IPv6
 * @param root pointer to tbm data
 * @param key ip address
 * @return rule of longest match
 */
_LPM_RULE lpm6_lookup(lpm6_root * root, struct in6_addr * key)
{
	return _tbm_lookup(root, (uint32_t * ) key);
}
