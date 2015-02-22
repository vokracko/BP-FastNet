#include "bspl.h"

//TODO delete když tam není add

_bspl_node * _bspl_root;
_bspl_node6 * _bspl_root6;

_bspl_node ** _bspl_htable;
_bspl_node6 ** _bspl_htable6;

inline uint32_t calculate_hash(uint32_t key)
{
	uint32_t hash, i;

	for(hash = i = 0; i < 4; ++i)
	{
		hash += GET_BYTE_LSB(key, i);
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
 * @param node
 */
inline void _bspl_add_htable(_bspl_node * node)
{
	uint32_t index = calculate_hash(node->prefix);
	_bspl_node * ptr = _bspl_htable[index];

	if(ptr == NULL)
	{
		_bspl_htable[index] = node;
	}
	else
	{
		while(ptr->next != NULL) ptr = ptr->next;
		ptr->next = node;
	}
}

/**
 * @brief Insert node into bspl tree structure
 * @param node [description]
 * @param parent parent node in tree structure
 * @param bit left or right descendant
 */
inline void _bspl_add_tree(_bspl_node * node, _bspl_node * parent, bool bit)
{
	// left => bit = 0; left is first in struct
	// right => bit = 1; right is second in struct
	*( (_bspl_node **) ((void *) parent + _BSPL_TREE_OFFSET(bit))) = node;
}

/**
 * @brief Insert node inside bspl tree and bspl hast table structure
 * @param node
 * @param parent parent node
 * @param bit left or right descendant
 */
inline void _bspl_add(_bspl_node * node, _bspl_node * parent, bool bit)
{
	_bspl_add_tree(node, parent, bit);
	_bspl_add_htable(node);
}


// TODO malloc na null
/**
 * @brief Construct empty bspl node
 * @return pointer to constructed node
 */
inline _bspl_node * _bspl_create()
{
	_bspl_node * node;
	node = (_bspl_node *) malloc(sizeof(_bspl_node));
	node->left = NULL;
	node->right = NULL;
	node->next = NULL;
	node->type = _BSPL_NODE_PREFIX;

	return node;
}

/**
 * @brief Remove node from hash table
 * @details Does not free memory used by node
 * @param node
 */
inline void _bspl_remove_htable(_bspl_node * node)
{
	unsigned int index = calculate_hash(node->prefix);
	_bspl_node * htable_node = _bspl_htable[index];

	// linked list contains only one node
	if(_bspl_htable[index] == node)
	{
		_bspl_htable[index] = node->next;
	}
	// desired node is somewhere else
	else
	{
		while(htable_node->next != node)
		{
			htable_node = htable_node->next;
		}

		htable_node->next = node->next;
	}
}

/**
 * @brief Remove node from bspl tree structure
 * @detail Does not free memory used by node
 * @param parent
 * @param bit left or right descendant
 */
inline void _bspl_remove_tree(_bspl_node * parent, bool bit)
{
	*((_bspl_node **) ((void *) parent + _BSPL_TREE_OFFSET(bit))) = NULL;
}

/**
 * @brief Remove node from all used strucures
 * @details Frees memory used by node
 * @param node
 * @param parent
 * @param bit left or right
 */
inline void _bspl_remove(_bspl_node * node, _bspl_node * parent, bool bit)
{
	_bspl_remove_tree(parent, bit);
	_bspl_remove_htable(node);

	free(node);
}

/**
 * @brief Search in bspl tree structure
 * @param prefix
 * @param prefix_len
 * @param parent[out]
 * @param other[out]
 * @return node
 */
_bspl_node * _bspl_lookup(uint32_t prefix, uint8_t prefix_len, _bspl_node ** parent, _bspl_node ** other)
{
	uint32_t bit_position = 31;
	uint32_t len = 0;
	bool bit;
	_bspl_node * node = _bspl_root;
	*parent = NULL;
	*other = NULL;

	do
	{
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
 * @param default_rule6 default rule for IPv6
 */
void lpm_init(_LPM_RULE default_rule, _LPM_RULE default_rule6)
{
	// INIT for IPv4
	_bspl_root = _bspl_create();
	_bspl_root->type = _BSPL_NODE_PREFIX;
	_bspl_root->rule = default_rule;
	_bspl_root->prefix = 0;
	_bspl_root->prefix_len = 0;

	_bspl_htable = (_bspl_node **) calloc(_BSPL_HTABLE_SIZE, sizeof(_bspl_node *));
	_bspl_add_htable(_bspl_root);

	// INIT for IPv6
	// _bspl_root6 = _bspl_create6();
	// _bspl_root6->type = _BSPL_NODE_PREFIX;
	// _bspl_root6->rule = default_rule6;
	// _bspl_root6->prefix = 0;

	// _bspl_htable6 = (_bspl_node6 **) calloc(_BSPL_HTABLE_SIZE, sizeof(_bspl_node6 *));
	// _bspl_add_htable6(_bspl_root6);
}


/**
 * @brief Free all data structures allocated for BSPL
 */
void lpm_destroy()
{
	_bspl_node * node, * prev;
	_bspl_node6 * node6, * prev6;
	// IPv4
	for(int i = 0; i < _BSPL_HTABLE_SIZE; ++i)
	{
		node = _bspl_htable[i];

		while(node != NULL)
		{
			prev = node;
			node = node->next;

			free(prev);
		}

		// node6 = _bspl_htable6[i];

		// while(node6 != NULL)
		// {
		// 	prev6 = node6;
		// 	node6 = node6->next;

		// 	free(prev6);
		// }

	}

	free(_bspl_htable);
	// free(_bspl_htable6);
}

// TODO bude vracet na základě mallocu?
//TODO bude vracet něco? pokud existuje/nepodařilo se vložit

/**
 * @brief Insert rule into bspl structures
 * @param prefix prefix of rule
 * @param prefix_len [description]
 * @param rule
 */
void lpm_add(uint32_t prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	_bspl_node* node = NULL;
	_bspl_node* parent = _bspl_root;
	_bspl_node* other;
	bool bit;
	uint8_t len = 0;
	uint32_t parent_rule;
	uint32_t prefix_bits;

	do
	{
		bit = GET_BIT_MSB(prefix, len);
		prefix_bits = GET_BITS_MSB(parent->prefix, len);
		parent_rule = parent->rule;
		node =  bit ? parent->right : parent->left;
		other  =  bit ? parent->left : parent->right;
		parent->type = _BSPL_NODE_INTERNAL;

		if(other == NULL)
		{
			other = _bspl_create();
			other->prefix = prefix_bits | (!bit << (31 - len));
			other->prefix_len = len + 1;
			other->rule = parent_rule;
			_bspl_add(other, parent, !bit);

		}

		if(node == NULL)
		{
			node = _bspl_create();
			node->prefix = prefix_bits | (bit << (31 - len));
			node->prefix_len = len + 1;
			node->rule = parent_rule;
			_bspl_add(node, parent, bit);
		}

		parent = node;
		++len;
	}
	while(prefix_len != len);

	_bspl_leaf_pushing(node, node->rule, rule);
}


/**
 * @brief Update rule for specified prefix
 * @param prefix
 * @param prefix_len
 * @param rule new rule
 */
void lpm_update(uint32_t prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	_bspl_node * parent, * other;
	_bspl_node * node = _bspl_lookup(prefix, prefix_len, &parent, &other);

	_bspl_leaf_pushing(node, node->rule, rule);
}

/**
 * @brief Remove node from all bspl structures
 * @param prefix
 * @param prefix_len
 */
void lpm_remove(uint32_t prefix, uint8_t prefix_len)
{
	// FIXME memleak
	_bspl_node * other, * parent;
	_bspl_node * node = _bspl_lookup(prefix, prefix_len, &parent, &other);

	// node is leaf node, other node is leaf node and does not contain different prefix from parent node (contructed by leaf-pushing)
	if(node->type == _BSPL_NODE_PREFIX && other->type == _BSPL_NODE_PREFIX && other->rule == parent->rule)
	{
		_bspl_remove(node, parent, parent->right == node);
		_bspl_remove(other, parent, parent->right == other);
		parent->type = _BSPL_NODE_PREFIX;
	}
	else
	{
		_bspl_leaf_pushing(node, node->rule, parent->rule);
	}
}

/**
 * @brief Search trought htable for longest match
 * @param key ip address
 * @return rule number
 * @todo vracet thread_id + další fce get lookup result?
 */
uint32_t lpm_lookup(uint32_t key)
{
	uint32_t prefix_bits; // extracted part of prefix
	uint8_t prefix_len = 32; // binary search actual length
	uint8_t prefix_change = 32; // binary search length change

	_bspl_node * node = NULL;

	do
	{
		prefix_bits = GET_BITS_MSB(key, prefix_len);
		node = _bspl_htable[calculate_hash(prefix_bits)];

		while(node != NULL && (node->prefix != prefix_bits || node->prefix_len != prefix_len))
		{
			node = node->next;
		}

		prefix_change >>= 1;

		if(node == NULL) prefix_len -= prefix_change;
		else if(node->type == _BSPL_NODE_INTERNAL) prefix_len += prefix_change;
		else break;

	} while(prefix_change > 0);

	if(node == NULL) return _bspl_root->rule;

	return node->rule;

}
