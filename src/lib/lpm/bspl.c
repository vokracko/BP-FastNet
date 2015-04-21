#include "bspl.h"

_bspl_node * _bspl_root;
_bspl_node ** _bspl_htable;

inline uint32_t calculate_hash(uint8_t * key, uint8_t length)
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
void _bspl_leaf_pushing(_bspl_node * node, _LPM_RULE rule_original, _LPM_RULE rule_new)
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
inline void _bspl_add_htable(lpm_root * root, _bspl_node * node)
{
	assert(root != NULL);
	assert(node != NULL);
	assert(root->htable != NULL);

	uint32_t index = calculate_hash((uint8_t *) &node->prefix, 4);
	_bspl_node ** ptr = &root->htable[index];

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
inline void _bspl_add_tree(_bspl_node * node, _bspl_node * parent, _Bool bit)
{
	assert(parent != NULL);
	assert(node != NULL);
	// left => bit = 0; left is first in struct
	// right => bit = 1; right is second in struct
	*( (_bspl_node **) ((void *) parent + _BSPL_TREE_OFFSET(bit))) = node;
}

/**
 * @brief Insert node inside bspl tree and bspl hast table structure
 * @param root
 * @param node
 * @param parent parent node
 * @param bit left or right descendant
 */
inline void _bspl_add(lpm_root * root, _bspl_node * node, _bspl_node * parent, _Bool bit)
{
	assert(root != NULL);
	assert(root->htable != NULL);
	assert(node != NULL);
	assert(parent != NULL);

	_bspl_add_tree(node, parent, bit);
	_bspl_add_htable(root, node);
}


/**
 * @brief Construct empty bspl node
 * @return pointer to constructed node
 */
inline void * _bspl_create(uint8_t size)
{
	_bspl_node * node;
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
inline void _bspl_remove_htable(lpm_root * root,_bspl_node * node)
{
	assert(root != NULL);
	assert(node != NULL);
	assert(root->htable != NULL);

	unsigned int index = calculate_hash((uint8_t *) &node->prefix, 4);
	_bspl_node ** htable_node = &root->htable[index];

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
inline void _bspl_remove_tree(_bspl_node * parent, _Bool bit)
{
	assert(parent != NULL);

	*((_bspl_node **) ((void *) parent + _BSPL_TREE_OFFSET(bit))) = NULL;
}

/**
 * @brief Remove node from all used strucures
 * @details Frees memory used by node
 * @param root
 * @param node
 * @param parent
 * @param bit left or right
 */
inline void _bspl_remove(lpm_root * root, _bspl_node * node, _bspl_node * parent, _Bool bit)
{
	assert(root != NULL);
	assert(root->htable != NULL);
	assert(node != NULL);
	assert(parent != NULL);

	_bspl_remove_tree(parent, bit);
	_bspl_remove_htable(root, node);

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
_bspl_node * _bspl_lookup(lpm_root * root, uint32_t prefix, uint8_t prefix_len, _bspl_node ** parent, _bspl_node ** other)
{
	assert(root != NULL);
	assert(root->tree != NULL);

	uint32_t bit_position = 31;
	uint32_t len = 0;
	_Bool bit;
	_bspl_node * node = root->tree;
	*parent = NULL;
	*other = NULL;

	do
	{
		assert(node != NULL);

		bit = GET_BIT_LSB(prefix, bit_position--);
		*parent = node;
		*other = *(_bspl_node **) ((void *) node + _BSPL_TREE_OFFSET(!bit));; // *other = bit ? node->left : node->right
		node = *(_bspl_node **) ((void *) node + _BSPL_TREE_OFFSET(bit)); // node = bit ? node->right : node->left
	}
	while(++len != prefix_len && node != NULL);

	return node;
}

/**
 * @brief Construct bspl structures
 * @param default_rule default rule for IPv4
 * @return bspl_root
 */
lpm_root * lpm_init(_LPM_RULE default_rule)
{
	lpm_root * root = malloc(sizeof(lpm_root));

	if(root == NULL) return errno = FASTNET_OUT_OF_MEMORY, NULL;
	root->tree = _bspl_create(sizeof(_bspl_node));
	root->tree->type = _BSPL_NODE_PREFIX;
	root->tree->rule = default_rule;
	root->tree->prefix = 0;
	root->tree->prefix_len = 0;

	root->htable = (_bspl_node **) calloc(_BSPL_HTABLE_SIZE, sizeof(_bspl_node *));
	_bspl_add_htable(root, root->tree);

	return root;
}


/**
 * @brief Free all data structures allocated for BSPL
 * @param root
 */
void lpm_destroy(lpm_root * root)
{
	if(root == NULL) return;
	assert(root->htable != NULL);

	_bspl_node * node, * prev;

	for(int i = 0; i < _BSPL_HTABLE_SIZE; ++i)
	{
		// IPv4
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

/**
 * @brief Insert rule into bspl structures
 * @param root
 * @param prefix prefix of rule
 * @param prefix_len [description]
 * @param rule
 * @todo bude vracet něco? pokud existuje/nepodařilo se vložit
 */
_Bool lpm_add(lpm_root * root, struct in_addr * prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	assert(root != NULL);
	assert(root->tree != NULL);

	_bspl_node* node = NULL;
	_bspl_node* parent = root->tree;
	_bspl_node* other;
	_Bool bit;
	uint8_t len = 0;
	uint32_t parent_rule;
	uint32_t prefix_bits;
	uint32_t prefix_int = (uint32_t) prefix->s_addr;

	do
	{
		assert(parent != NULL);

		bit = GET_BIT_MSB(prefix_int, len);
		prefix_bits = GET_BITS_MSB(parent->prefix, len);
		parent_rule = parent->rule;
		node =  bit ? parent->right : parent->left;
		other  =  bit ? parent->left : parent->right;
		parent->type = _BSPL_NODE_INTERNAL;

		if(other == NULL)
		{
			other = _bspl_create(sizeof(_bspl_node));

			if(other == NULL) return errno = FASTNET_OUT_OF_MEMORY, 0;

			other->prefix = prefix_bits | (!bit << (31 - len));
			other->prefix_len = len + 1;
			other->rule = parent_rule;
			_bspl_add(root, other, parent, !bit);

		}

		if(node == NULL)
		{
			node = _bspl_create(sizeof(_bspl_node));

			if(node == NULL) return errno = FASTNET_OUT_OF_MEMORY, 0;
			node->prefix = prefix_bits | (bit << (31 - len));
			node->prefix_len = len + 1;
			node->rule = parent_rule;
			_bspl_add(root, node, parent, bit);
		}

		parent = node;
		++len;
	}
	while(prefix_len != len);

	_bspl_leaf_pushing(node, node->rule, rule);

	return 1;
}


/**
 * @brief Update rule for specified prefix
 * @param root
 * @param prefix
 * @param prefix_len
 * @param rule new rule
 */
void lpm_update(lpm_root * root, struct in_addr * prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	assert(root != NULL);

	_bspl_node * parent, * other;
	_bspl_node * node = _bspl_lookup(root, (uint32_t) prefix->s_addr, prefix_len, &parent, &other);

	assert(node != NULL);

	_bspl_leaf_pushing(node, node->rule, rule);
}

/**
 * @brief Remove node from all bspl structures
 * @param root
 * @param prefix
 * @param prefix_len
 */
void lpm_remove(lpm_root * root, struct in_addr * prefix, uint8_t prefix_len)
{
	assert(root != NULL);

	_bspl_node * other, * parent;
	_bspl_node * node = _bspl_lookup(root, (uint32_t) prefix->s_addr, prefix_len, &parent, &other);

	// node is leaf node, other node is leaf node and does not contain different prefix from parent node (contructed by leaf-pushing)

	assert(node != NULL);
	assert(other != NULL);

	if(node->type == _BSPL_NODE_PREFIX && other->type == _BSPL_NODE_PREFIX && other->rule == parent->rule)
	{
		assert(parent != NULL);

		_bspl_remove(root, node, parent, parent->right == node);
		_bspl_remove(root, other, parent, parent->right == other);
		parent->type = _BSPL_NODE_PREFIX;
	}
	else
	{
		assert(node != NULL);

		_bspl_leaf_pushing(node, node->rule, parent->rule);
	}
}

/**
 * @brief Search trought htable for longest match
 * @param root
 * @param key ip address
 * @return rule number
 */
_LPM_RULE lpm_lookup(lpm_root * root, struct in_addr * key)
{
	assert(root != NULL);
	assert(root->htable != NULL);

	uint32_t prefix_bits; // extracted part of prefix
	uint8_t prefix_len = 32; // binary search actual length
	uint8_t prefix_change = 32; // binary search length change
	uint32_t key_int = (uint32_t) key->s_addr;
	_bspl_node * node = NULL;

	do
	{
		prefix_bits = GET_BITS_MSB(key_int, prefix_len);
		node = root->htable[calculate_hash((uint8_t *) &prefix_bits, 4)];

		while(node != NULL && (node->prefix != prefix_bits || node->prefix_len != prefix_len))
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

lpm6_root * lpm6_init(_LPM_RULE default_rule)
{
	return NULL;
}

_Bool lpm6_add(lpm6_root * root, struct in6_addr * prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	return 1;
}

void lpm6_update(lpm6_root * root, struct in6_addr * prefix, uint8_t prefix_len, _LPM_RULE rule)
{

}

void lpm6_remove(lpm6_root * root, struct in6_addr * prefix, uint8_t prefix_len)
{

}

_Bool lpm6_destroy(lpm6_root * root)
{
	return 1;
}

_LPM_RULE lpm6_lookup(lpm6_root * root, struct in6_addr * key)
{
	return 1;
}
