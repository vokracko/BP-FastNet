#include "bspl.h"

uint32_t calculate_hash(uint8_t * key, uint8_t length)
{
	uint32_t hash, i;

	for(hash = i = 0; i < length; ++i)
	{
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash % _BSPL_HTABLE_SIZE;
}

/**
 * @brief Push rule_new to all descendants with rule original
 * @param node start node
 * @param rule_original
 * @param rule_new
 */
void _bspl_leaf_pushing(_bspl_node_common * node, _LPM_RULE rule_original, _LPM_RULE rule_new)
{
	assert(node != NULL);
	assert(rule_original != rule_new);

	if(node->rule == rule_original)
	{
		node->rule = rule_new;
	}

	if(node->type == _BSPL_NODE_INTERNAL)
	{
		_bspl_leaf_pushing(node->left, rule_original, rule_new);
		_bspl_leaf_pushing(node->right, rule_original, rule_new);
	}
}

/**
 * @brief Insert node into bspl hash table structure
 * @param root
 * @param node
 */
void _bspl_add_htable(lpm_root * root, _bspl_node_common * node, uint8_t bytes_of_prefix)
{
	assert(root != NULL);
	assert(node != NULL);
	assert(root->htable != NULL);

	uint32_t index = calculate_hash((uint8_t *) &((_bspl_node *)node)->prefix, bytes_of_prefix);
	printf("%d index, %d length\n", index, node->prefix_len);
	_bspl_node_common ** ptr = &root->htable[index];

	while(*ptr != NULL)
	{
		ptr = &((*ptr)->next);
	}

	*ptr = node;
}

/**
 * @brief Insert node into bspl tree structure
 * @param node [description]
 * @param parent parent node in tree structure
 * @param bit left or right descendant
 */
void _bspl_add_tree(_bspl_node_common * node, _bspl_node_common * parent, _Bool bit)
{
	assert(parent != NULL);
	assert(node != NULL);
	// left => bit = 0; left is first in struct
	// right => bit = 1; right is second in struct
	*( (_bspl_node_common **) ((void *) parent + _BSPL_TREE_OFFSET(bit))) = node;
}

/**
 * @brief Insert node inside bspl tree and bspl hast table structure
 * @param root
 * @param node
 * @param parent parent node
 * @param bit left or right descendant
 */
void _bspl_add_node(lpm_root * root, _bspl_node_common * node, _bspl_node_common * parent, _Bool bit, uint8_t bytes_of_prefix)
{
	assert(root != NULL);
	assert(root->htable != NULL);
	assert(node != NULL);
	assert(parent != NULL);

	_bspl_add_tree(node, parent, bit);
	_bspl_add_htable(root, node, bytes_of_prefix);
}


/**
 * @brief Construct empty bspl node
 * @return pointer to constructed node
 */
void * _bspl_create(uint8_t size)
{
	_bspl_node_common * node;
	node = malloc(size);

	if(node == NULL) return errno = FASTNET_OUT_OF_MEMORY, NULL;
	node->left = NULL;
	node->right = NULL;
	node->next = NULL;
	node->type = _BSPL_NODE_PREFIX;

	return node;
}

/**
 * @brief Remove node from hash table
 * @details Does not free memory used by node
 * @param root
 * @param node
 */
void _bspl_remove_htable(lpm_root * root, _bspl_node_common * node, uint8_t bytes_of_prefix)
{
	assert(root != NULL);
	assert(node != NULL);
	assert(root->htable != NULL);

	unsigned int index = calculate_hash((uint8_t *) &((_bspl_node *)node)->prefix, bytes_of_prefix);
	_bspl_node_common ** htable_node = &root->htable[index];

	while(*htable_node != node)
	{
		htable_node = &((*htable_node)->next);
	}

	*htable_node = (*htable_node)->next;
}

/**
 * @brief Remove node from bspl tree structure
 * @detail Does not free memory used by node
 * @param parent
 * @param bit left or right descendant
 */
void _bspl_remove_tree(_bspl_node_common * parent, _Bool bit)
{
	assert(parent != NULL);

	*((_bspl_node_common **) ((void *) parent + _BSPL_TREE_OFFSET(bit))) = NULL;
}

/**
 * @brief Remove node from all used strucures
 * @details Frees memory used by node
 * @param root
 * @param node
 * @param parent
 * @param bit left or right
 */
void _bspl_remove_node(lpm_root * root, _bspl_node_common * node, _bspl_node_common * parent, _Bool bit, uint8_t bytes_of_prefix)
{
	assert(root != NULL);
	assert(root->htable != NULL);
	assert(node != NULL);
	assert(parent != NULL);

	_bspl_remove_tree(parent, bit);
	_bspl_remove_htable(root, node, bytes_of_prefix);

	free(node);
}

/**
 * @brief Search in bspl tree structure
 * @param root
 * @param prefix
 * @param prefix_len
 * @param parent[out]
 * @param other[out]
 * @return node
 */
_bspl_node_common * _bspl_lookup_internal(_bspl_node_common * node, uint32_t * prefix, uint8_t prefix_len, _bspl_node_common ** parent, _bspl_node_common ** other)
{
	assert(node != NULL);

	uint32_t bit_position = 0;
	uint32_t len = 0;
	_Bool bit;
	*parent = NULL;
	*other = NULL;

	do
	{
		assert(node != NULL);

		bit = GET_BIT_MSB(prefix, bit_position);
		++bit_position;
		*parent = node;
		*other = *(_bspl_node_common **) ((void *) node + _BSPL_TREE_OFFSET(!bit));; // *other = bit ? node->left : node->right
		node = *(_bspl_node_common **) ((void *) node + _BSPL_TREE_OFFSET(bit)); // node = bit ? node->right : node->left
	}
	while(++len != prefix_len && node != NULL);

	return node;
}

lpm_root * _bspl_init(_LPM_RULE default_rule, uint8_t size, uint32_t bytes_of_prefix)
{
	lpm_root * root = malloc(sizeof(lpm_root));

	if(root == NULL) return errno = FASTNET_OUT_OF_MEMORY, NULL;
	root->tree = _bspl_create(size);
	root->tree->type = _BSPL_NODE_PREFIX;
	root->tree->rule = default_rule;
	root->tree->prefix_len = 0;

	memset(&(((_bspl_node *)root->tree)->prefix), 0, bytes_of_prefix);

	root->htable = (_bspl_node_common **) calloc(_BSPL_HTABLE_SIZE, sizeof(_bspl_node_common *));

	if(root->htable == NULL) return errno = FASTNET_OUT_OF_MEMORY, free(root), NULL;
	_bspl_add_htable(root, root->tree, bytes_of_prefix);

	return root;
}

void _bspl_destroy(lpm_root * root)
{
	if(root == NULL) return;
	assert(root->htable != NULL);

	_bspl_node_common * node, * prev;

	for(int i = 0; i < _BSPL_HTABLE_SIZE; ++i)
	{
		node = root->htable[i];

		while(node != NULL)
		{
			prev = node;
			node = node->next;

			free(prev);
		}
	}

	free(root->htable);
	free(root);
}

uint32_t * _bspl_get_bits(uint32_t * key, uint8_t length, uint8_t bytes_of_prefix)
{
	static uint32_t addr[4];
	memcpy(addr, key, bytes_of_prefix);

	for(unsigned i = (length - 1) / 32 + 1; i < bytes_of_prefix / 4; ++i)
	{
		addr[i] = 0;
	}

	if(length%32 != 0)
	{
		addr[(length - 1)/ 32] &= (~0 << (32 - ( length % 32)));
		printf("length %d, shift index %d, shift %d\n", length, (length - 1)/ 32, 32 - ( length % 32));
	}
	else
	{
		printf("length %d, set index %d, skip shift\n", length, (length - 1)/ 32 + 1);
	}

	return addr;
}

void _bspl_set_bits(uint32_t * dest, uint32_t * src, _Bool bit, uint8_t len, uint8_t bytes_of_prefix)
{
	memcpy(dest, src, bytes_of_prefix);
	if(bit) SET_BIT_MSB(dest[(len-1) / 32], len % 32);
	else CLEAR_BIT_MSB(dest[(len-1) / 32], len % 32);

}


_LPM_RULE _bspl_lookup(lpm_root * root, uint32_t * key, uint8_t length, uint8_t bytes_of_prefix)
{
	assert(root != NULL);
	assert(root->htable != NULL);

	uint32_t * prefix_bits; // extracted part of prefix
	uint8_t prefix_len = length; // binary search actual length
	uint8_t prefix_change = length; // binary search length change
	_bspl_node_common * node = NULL;
	_bspl_node_common * dummy = NULL;
	node = _bspl_lookup_internal(root->tree, key, 33, &dummy, &dummy);
	_bspl_node6 * node6 = ((_bspl_node6*)node);
	prefix_bits = _bspl_get_bits(key, 33, bytes_of_prefix);
	printf("node %u, key %u\n", ((uint32_t *) &(node6->prefix))[0], prefix_bits[0]);
	printf("node %u, key %u\n", ((uint32_t *) &(node6->prefix))[1], prefix_bits[1]);
	printf("node %u, key %u\n", ((uint32_t *) &(node6->prefix))[2], prefix_bits[2]);
	printf("node %u, key %u\n", ((uint32_t *) &(node6->prefix))[3], prefix_bits[3]);
	printf("memcmp = %d\n", memcmp(&(((_bspl_node *)node)->prefix), prefix_bits, 4));

	do
	{
		prefix_bits = _bspl_get_bits(key, prefix_len, bytes_of_prefix);
		node = root->htable[calculate_hash((uint8_t *) prefix_bits, bytes_of_prefix)];

		while(node != NULL && (node->prefix_len != prefix_len || memcmp(&(((_bspl_node *)node)->prefix), prefix_bits, bytes_of_prefix)!= 0))
		{
			node = node->next;
		}

		prefix_change >>= 1;

		if(node == NULL) prefix_len -= prefix_change;
		else if(node->type == _BSPL_NODE_INTERNAL) prefix_len += prefix_change;
		else break;

	} while(prefix_change > 0);

	if(node == NULL) return root->tree->rule;

	return node->rule;
}

void _bspl_remove(lpm_root * root, uint32_t * prefix, uint8_t prefix_len, uint8_t size)
{
	assert(root != NULL);

	_bspl_node_common * other, * parent;
	_bspl_node_common * node = _bspl_lookup_internal(root->tree, prefix, prefix_len, &parent, &other);

	// node is leaf node, other node is leaf node and does not contain different prefix from parent node (contructed by leaf-pushing)

	assert(node != NULL);
	assert(other != NULL);

	if(node->type == _BSPL_NODE_PREFIX && other->type == _BSPL_NODE_PREFIX && other->rule == parent->rule)
	{
		assert(parent != NULL);

		_bspl_remove_node(root, node, parent, parent->right == node, size);
		_bspl_remove_node(root, other, parent, parent->right == other, size);
		parent->type = _BSPL_NODE_PREFIX;
	}
	else
	{
		assert(node != NULL);

		_bspl_leaf_pushing(node, node->rule, parent->rule);
	}
}

/**
 * @brief Update rule for specified prefix
 * @param root
 * @param prefix
 * @param prefix_len
 * @param rule new rule
 */
void _bspl_update(lpm_root * root, uint32_t * prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	assert(root != NULL);

	_bspl_node_common * parent, * other;
	_bspl_node_common * node = _bspl_lookup_internal(root->tree, prefix, prefix_len, &parent, &other);

	assert(node != NULL);

	_bspl_leaf_pushing(node, node->rule, rule);
}

/**
 * @brief Insert rule into bspl structures
 * @param root
 * @param prefix prefix of rule
 * @param prefix_len [description]
 * @param rule
 */
_Bool _bspl_add(lpm_root * root, uint32_t * prefix, uint8_t prefix_len, _LPM_RULE rule, uint8_t size, uint8_t bytes_of_prefix)
{
	assert(root != NULL);
	assert(root->tree != NULL);

	_bspl_node_common * node = root->tree;
	_bspl_node_common * parent = root->tree;
	_bspl_node_common * other;
	_Bool bit;
	uint8_t len = 0;
	uint32_t parent_rule;
	uint32_t * prefix_bits;

	do
	{
		assert(parent != NULL);

		bit = GET_BIT_MSB(prefix, len);
		prefix_bits = _bspl_get_bits(&(((_bspl_node *) node)->prefix), len, bytes_of_prefix);
		parent_rule = parent->rule;
		node =  bit ? parent->right : parent->left;
		other  =  bit ? parent->left : parent->right;
		parent->type = _BSPL_NODE_INTERNAL;


		if(other == NULL)
		{
			other = _bspl_create(size);

			if(other == NULL) return errno = FASTNET_OUT_OF_MEMORY, 0;
			_bspl_set_bits(&(((_bspl_node *) other)->prefix), prefix_bits, !bit, len, bytes_of_prefix);
			other->prefix_len = len + 1;
			other->rule = parent_rule;
			_bspl_add_node(root, other, parent, !bit, bytes_of_prefix);

		}

		if(node == NULL)
		{
			node = _bspl_create(size);

			if(node == NULL) return errno = FASTNET_OUT_OF_MEMORY, 0;
			_bspl_set_bits(&(((_bspl_node *) node)->prefix), prefix_bits, bit, len, bytes_of_prefix);
			node->prefix_len = len + 1;
			node->rule = parent_rule;
			_bspl_add_node(root, node, parent, bit, bytes_of_prefix);
		}

		parent = node;
		++len;
	}
	while(prefix_len != len);

	_bspl_leaf_pushing(node, node->rule, rule);

	return 1;
}

/**
 * @brief Construct bspl structures
 * @param default_rule default rule for IPv4
 * @return bspl_root
 */
lpm_root * lpm_init(_LPM_RULE default_rule)
{
	return _bspl_init(default_rule, sizeof(_bspl_node), 4);
}

lpm_root * lpm6_init(_LPM_RULE default_rule)
{
	return _bspl_init(default_rule, sizeof(_bspl_node6), 16);
}

/**
 * @brief Free all data structures allocated for BSPL
 * @param root
 */
void lpm_destroy(lpm_root * root)
{
	_bspl_destroy(root);
}

void lpm6_destroy(lpm_root * root)
{
	_bspl_destroy(root);
}

/**
 * @brief Remove node from all bspl structures
 * @param root
 * @param prefix
 * @param prefix_len
 */
 void lpm_remove(lpm_root * root, struct in_addr * prefix, uint8_t prefix_len)
 {
 	_bspl_remove(root, (uint32_t *) prefix, prefix_len, 4);
 }

/**
 * @brief Remove node from all bspl structures
 * @param root
 * @param prefix
 * @param prefix_len
 */
void lpm6_remove(lpm_root * root, struct in6_addr * prefix, uint8_t prefix_len)
{
	_bspl_remove(root, (uint32_t *) prefix, prefix_len, 16);
}

void lpm_update(lpm_root * root, struct in_addr * prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	_bspl_update(root, (uint32_t *) prefix, prefix_len, rule);
}

void lpm6_update(lpm_root * root, struct in6_addr * prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	_bspl_update(root, (uint32_t *) prefix, prefix_len, rule);
}
/**
 * @brief Search trought htable for longest match
 * @param root
 * @param key ip address
 * @return rule number
 */
_LPM_RULE lpm_lookup(lpm_root * root, struct in_addr * key)
{
	return _bspl_lookup(root, (uint32_t *) key, 32, 4);
}

_LPM_RULE lpm6_lookup(lpm_root * root, struct in6_addr * key)
{
	return _bspl_lookup(root, (uint32_t *) key, 128, 16);
}

_Bool lpm_add(lpm_root * root, struct in_addr * prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	return _bspl_add(root, (uint32_t *) prefix, prefix_len, rule, sizeof(_bspl_node), 4);
}

_Bool lpm6_add(lpm_root * root, struct in6_addr * prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	return _bspl_add(root, (uint32_t *) prefix, prefix_len, rule, sizeof(_bspl_node6), 16);
}
