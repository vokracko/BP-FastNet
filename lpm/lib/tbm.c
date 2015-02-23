#include "tbm.h"


_tbm_node * _tbm_root;

/**
 * @brief Count all ones up to given position
 * @param key
 * @param bit_position stop marker
 * @return number of ones in key
 */
inline uint8_t _tbm_bitsum(uint32_t key, uint8_t bit_position)
{
	// cut off the bits at bit position and after that
	key = GET_BITS_MSB(key, bit_position - 1);
	return __builtin_popcount(key);
}

/**
 * @brief Calculate index for rule based on length of match, used only for lookup
 * @param bit_vector internal bitmap
 * @param bit_value part of ip address
 * @return index to node->rule
 */
inline uint8_t _tbm_internal_index(uint32_t bit_vector, uint8_t bit_value)
{
	int8_t length = STRIDE;
	uint8_t bit_position;

	// from longest match to shortest: NNN, NN*, N*, *
	do
	{
		bit_position = (1 << length) - 1 + bit_value;
		bit_value >>= 1;
	}
	while(length-- >= 0 && GET_BIT_MSB(bit_vector, bit_position) == 0);

	return _tbm_bitsum(bit_vector, bit_position);
}

/**
 * @brief Lookup node where prefix is located, used by update/remove
 * @param prefix
 * @param prefix_len
 * @param index [out] int value of prefix part which is used as index to internal/external
 * @return node
 */
inline _tbm_node * _tbm_lookup(uint32_t prefix, uint8_t prefix_len, uint8_t * index)
{
	uint8_t bit_value, length;
	uint8_t position = 0;
	_tbm_node * node = _tbm_root;

	bit_value = GET_STRIDE_BITS(prefix, position, STRIDE);

	while(++position * STRIDE < prefix_len)
	{
		bit_value = GET_STRIDE_BITS(prefix, position - 1, STRIDE);

		if(node->child == NULL); // TODO mazání prefixu jež neexistuje, jak se zachovat?

		node = &(node->child[_tbm_bitsum(node->external, bit_value)]);
	}

	bit_value = GET_STRIDE_BITS(prefix, position - 1, prefix_len - STRIDE * (position - 1));
	length = prefix_len - ((position - 1) * STRIDE);

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
inline void _tbm_extend(_tbm_node * node, uint8_t bit_value, bool shift_child)
{
	uint8_t bitsum;
	uint8_t index_start;
	uint32_t bitmap = shift_child ? node->external : node->internal;

	bitsum = _tbm_bitsum(bitmap, _TBM_ALL);
	index_start = _tbm_bitsum(bitmap, bit_value);

	if(shift_child)
	{
		//resize, add space for one more child
		node->child = realloc(node->child, (bitsum + 1) * sizeof(_tbm_node));
		//shift all values behind newly added to the right
		memmove(&(node->child[index_start + 1]), &(node->child[index_start]), (bitsum - index_start) * sizeof(_tbm_node));

		node->child[index_start].child = NULL;
		node->child[index_start].rule = NULL;
		node->child[index_start].external = 0;
		node->child[index_start].internal = 0;
	}
	else
	{
		//resize, add space for one more rule
		node->rule = realloc(node->rule, (bitsum + 1) * sizeof(_LPM_RULE));
		//shift all values behind newly added to the right
		memmove(&(node->rule[index_start + 1]), &(node->rule[index_start]), (bitsum - index_start) * sizeof(_LPM_RULE));
	}
}

/**
 * @brief Reduce array by 1
 * @param node
 * @param bit_value
 * @param shift_child 1 => change size of node->child otherwise node->rule
 * @todo implementovat mazání celých větví které na konci obsahují pouze jeden prefix? bude nutné procházet všechny potomky
 */
inline void _tbm_reduce(_tbm_node * node, uint8_t bit_value, bool shift_child)
{
	uint8_t bitsum;
	uint8_t index_start;
	uint32_t bitmap = shift_child ? node->external : node->internal;

	bitsum = _tbm_bitsum(bitmap, _TBM_ALL);
	index_start = _tbm_bitsum(bitmap, bit_value);

	if(shift_child)
	{
		memmove(&(node->child[index_start + 1]), &(node->child[index_start]), (bitsum - index_start - 1) * sizeof(_tbm_node));
		node->child = realloc(node->child, (bitsum - 1) * sizeof(_tbm_node));
	}
	else
	{
		memmove(&(node->rule[index_start + 1]), &(node->rule[index_start]), (bitsum - index_start - 1) * sizeof(_LPM_RULE));
		node->rule = realloc(node->rule, (bitsum - 1) * sizeof(_LPM_RULE));
	}
}

/**
 * @brief Destroy node children and deallocate memory
 * @param node
 */
void _tbm_destroy(_tbm_node * node)
{
	if(node->rule != NULL)
	{
		free(node->rule);
	}

	if(node->child != NULL)
	{
		for(int i = 0; i < _tbm_bitsum(node->external, _TBM_ALL); ++i)
		{
			_tbm_destroy(&(node->child[i]));
		}

		free(node->child);
	}
}


void lpm_init(_LPM_RULE default_rule, _LPM_RULE default_rule6)
{
	_tbm_root = malloc(sizeof(_tbm_node));
	_tbm_root->child = NULL;
	_tbm_root->rule = calloc(_TBM_SIZE_INTERNAL, sizeof(_LPM_RULE));
	_tbm_root->rule[0] = default_rule;
	_tbm_root->external = 0;
	_tbm_root->internal = 0;
	SET_BIT_MSB(_tbm_root->internal, 0);
}

void lpm_add(uint32_t prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	uint8_t position = 0;
	uint8_t index;
	uint8_t bit_value;
	uint8_t stride_len;

	_tbm_node * node = _tbm_root;

	while(++position * STRIDE < prefix_len)
	{
		bit_value = GET_STRIDE_BITS(prefix, position - 1, STRIDE);

		if(GET_BIT_MSB(node->external, bit_value) == 0)
		{
			_tbm_extend(node, bit_value, true);
			SET_BIT_MSB(node->external, bit_value);
		}

		node = &(((_tbm_node *) node->child)[_tbm_bitsum(node->external, bit_value)]);
	}

	stride_len = prefix_len - (position - 1) * STRIDE;
	//----------------------- number of bits used at this level <0, STRIDE>, value of those bits -------------
	index = INTERNAL_INDEX(stride_len, GET_STRIDE_BITS(prefix, position - 1, stride_len));

	_tbm_extend(node, index, false);
	SET_BIT_MSB(node->internal, index);
	node->rule[_tbm_bitsum(node->internal, index)] = rule;
}

void lpm_update(uint32_t prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	uint8_t index;
	_tbm_node * node = _tbm_lookup(prefix, prefix_len, &index);

	node->rule[_tbm_bitsum(node->internal, index)] = rule;
}

void lpm_remove(uint32_t prefix, uint8_t prefix_len)
{
	uint8_t index;
	_tbm_node * node = _tbm_lookup(prefix, prefix_len, &index);


	_tbm_reduce(node, index, false);
	CLEAR_BIT_MSB(node->internal, index);
}

void lpm_destroy()
{
	_tbm_destroy(_tbm_root);
	free(_tbm_root);
}

uint32_t lpm_lookup(uint32_t key)
{
	_tbm_node * node = _tbm_root;
	_tbm_node * parent = NULL; //TODO nějak ošetřit
	_tbm_node * longest_match_node = _tbm_root;
	uint8_t position = 0;
	uint8_t bits;
	uint8_t index;
	uint8_t longest_match_index = 0;


	// while there is longer match
	do
	{
		bits = GET_STRIDE_BITS(key, position++, STRIDE);

		if(node->rule != NULL)
		{
			longest_match_index = _tbm_internal_index(node->internal, bits);
			longest_match_node = node;
		}

		if(node->child == NULL) break;

		// index for child pointer is bitsum from <0, (int) bits)
		index = _tbm_bitsum(node->external, bits);

		// move to child
		parent = node;
		node = &(node->child[index]);
	}
	// longest_match_node is parent from current node (node contains child pointer from node operated in code above)
	while(GET_BIT_MSB(parent->external, bits));

	return longest_match_node->rule[longest_match_index];
}
