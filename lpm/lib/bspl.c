#include "bspl.h"

_bspl_node * _bspl_root;
_bspl_node6 * _bspl_root6;

_bspl_node ** _bspl_htable;
_bspl_node6 ** _bspl_htable6;

uint32_t calculate_hash(uint32_t key)
{
	uint32_t hash, i;

	for(hash = i = 0; i < 4; ++i)
	{
		hash += get_byte(key, i);
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash % _BSPL_HTABLE_SIZE;
}

// TODO přepsat na iteraci, morris průchod stromem??
/**
 * @brief Push rule_new to all descendants with rule original
 * @param node start node
 * @param rule_original
 * @param rule_new
 */
void _bspl_leaf_pushing(_bspl_node * node, _BSPL_RULE rule_original, _BSPL_RULE rule_new)
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
	*(parent + _BSPL_TREE_OFFSET(bit)) = node;
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
	node->type = _BSPL_NODE_CREATED;

	return node;
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
	if(_bspl_htable[index] == node && node->next == NULL)
	{
		_bspl_htable[index] = NULL;
	}
	// first item of list is desired node
	else if(_bspl_htable[index] == node)
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
	*(parent + _BSPL_TREE_OFFSET(bit)) = NULL;
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
		bit = get_bit(prefix, bit_position--);
		*parent = node;

		node = (node + _BSPL_TREE_OFFSET(bit)); // node = bit ? node->right : node->left
		*other = (node + _BSPL_TREE_OFFSET(!bit)); // *other = bit ? node->left : node->right

		++len;

	} while(len != prefix_len && node != NULL);

	return node;
}

/**
 * @brief Construct bspl structures
 * @param default_rule default rule for IPv4
 * @param default_rule6 default rule for IPv6
 */
void lpm_init(_BSPL_RULE default_rule, _BSPL_RULE default_rule6)
{
	// INIT for IPv4
	_bspl_root = (_bspl_node *) malloc(sizeof(_bspl_node));
	_bspl_root->type = _BSPL_NODE_PREFIX;
	_bspl_root->rule = default_rule;
	_bspl_root->left = NULL;
	_bspl_root->right = NULL;
	_bspl_root->next = NULL;
	_bspl_root->prefix = 0;

	_bspl_htable = (_bspl_node **) calloc(_BSPL_HTABLE_SIZE, sizeof(_bspl_node *));
	_bspl_add_htable(_bspl_root);
	_bspl_root->left = NULL;
	_bspl_root->right = NULL;
	_bspl_root->next = NULL;
	_bspl_root->prefix = 0;

	// INIT for IPv6
	// _bspl_root6 = (_bspl_node6 *) malloc(sizeof(_bspl_node6));
	// _bspl_root6->type = _BSPL_NODE_PREFIX;
	// _bspl_root6->rule = default_rule6;
	// _bspl_root6->left = NULL;
	// _bspl_root6->right = NULL;
	// _bspl_root6->next = NULL;
	// _bspl_root6->prefix = 0;

	// _bspl_htable6 = (_bspl_node6 **) calloc(_BSPL_HTABLE_SIZE, sizeof(_bspl_node6 *));
	// _bspl_node6_htable_add(_bspl_root6); //TODO
}


/**
 * @brief Free all data structures allocated for BSPL
 */
void lpm_destroy()
{
	_bspl_node * current, * prev;
	_bspl_node6 * current6, * prev6;
	// IPv4
	for(int i = 0; i < _BSPL_HTABLE_SIZE; ++i)
	{
		current = _bspl_htable[i];

		while(current != NULL)
		{
			prev = current;
			current = current->next;

			free(prev);
		}

		// current6 = _bspl_htable6[i];

		// while(current6 != NULL)
		// {
		// 	prev6 = current6;
		// 	current6 = current6->next;

		// 	free(prev6);
		// }

	}

	free(_bspl_htable);
	// free(_bspl_htable6); //TODO
}

// TODO bude vracet na základě mallocu?
//TODO bude vracet něco? pokud existuje/nepodařilo se vložit

/**
 * @brief Insert rule into bspl structures
 * @param prefix prefix of rule
 * @param prefix_len [description]
 * @param rule
 */
void lpm_add(uint32_t prefix, uint8_t prefix_len, _BSPL_RULE rule)
{
	_bspl_node* current = NULL;
	_bspl_node* parent;
	_bspl_node* other;
	bool bit;
	uint8_t len = 0;
	uint32_t parent_rule;
	uint32_t prefix_bits;

	parent = _bspl_root;

	do
	{
		bit = get_bit(prefix, 31 - len);
		prefix_bits = get_bits(parent->prefix, len);
		parent_rule = parent->rule;
		current =  bit ? parent->right : parent->left;
		other  =  bit ? parent->left : parent->right;
		parent->type = _BSPL_NODE_INTERNAL;

		if(other == NULL)
		{
			other = _bspl_create();
			other->type = _BSPL_NODE_PREFIX;
			other->prefix = prefix_bits | (!bit << (31 - len));
			other->prefix_len = len + 1;
			other->rule = parent_rule;
			_bspl_add(parent, other, !bit);

		}

		if(current == NULL)
		{
			current = _bspl_create();
			current->prefix = prefix_bits | (bit << (31 - len));
			current->prefix_len = len + 1;
			current->rule = parent_rule;
			_bspl_add(parent, current, bit);

			if(prefix_len == len + 1)
			{
				break;
			}

		}

		if(parent->type != _BSPL_NODE_INTERNAL) parent->type = _BSPL_NODE_INTERNAL;

		parent = current;
		++len;
	}
	while(prefix_len != len);

	if(current->type == _BSPL_NODE_CREATED)
	{
		current->type = _BSPL_NODE_PREFIX;
		current->rule = rule;
		current->prefix = get_bits(prefix, prefix_len);
		current->prefix_len = prefix_len;
	}
	else
	{
		_bspl_leaf_pushing(current, current->rule, rule);
	}
}


/**
 * @brief Update rule for specified prefix
 * @param prefix
 * @param prefix_len
 * @param rule new rule
 */
void lpm_update(uint32_t prefix, uint8_t prefix_len, _BSPL_RULE rule)
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
	_bspl_node * other, * parent;
	_bspl_node * node = _bspl_lookup(prefix, prefix_len, &parent, &other);

	// node is leaf node, other node is leaf node and does not contain different prefix from parent node (contructed by leaf-pushing)
	if(node->type == _BSPL_NODE_PREFIX && other->type == _BSPL_NODE_PREFIX && other->rule == parent->rule)
	{
		_bspl_remove(node, parent, 0);
		_bspl_remove(other, parent, 1);
	}
	else
	{
		_bspl_leaf_pushing(node, node->rule, parent->rule);
	}
}

void * _bspl_lookup_thread(void * key_ptr)
{
	uint32_t key = * (uint32_t *) key_ptr;
	uint32_t prefix_bits; // extracted part of prefix
	uint8_t prefix_len = 32; // binary search actual length
	uint8_t prefix_change = 32; // binary search length change

	_bspl_node * node = NULL;

	do
	{
		prefix_bits = get_bits(key, prefix_len);
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

	* (uint32_t *) key_ptr = node->rule;

	pthread_exit(NULL);
}

// // hledá v tabulce algoritmem BSPL
// TODO vrace thread_id + další fce get lookup result?
/**
 * @brief Search trought bspl htable for longest match
 */
uint32_t lpm_lookup(uint32_t key)
{
	pthread_t thread_id;

	// TODO -pthread kompilace
	pthread_create(&thread_id, NULL, _bspl_lookup_thread, (void *) &key);
	pthread_join(thread_id, NULL);

	return key;

}
