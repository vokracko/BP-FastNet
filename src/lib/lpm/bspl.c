#include "bspl.h"

uint32_t ip(uint32_t i, char * op)
{
	char dst[100];
	char * ptr;
	i = htonl(i);
	ptr = inet_ntop(AF_INET, &i, dst, 100);
	if(ptr == NULL) exit(26);
	// printf("%s - %u %s\n", op, i, ptr);
	return i;
}

uint32_t calculate_hash(uint32_t key)
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
void _bspl_add_htable(lpm_root * root, _bspl_node * node)
{
	assert(root != NULL);
	assert(node != NULL);
	assert(root->htable != NULL);
	uint32_t index = calculate_hash(node->prefix);
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
void _bspl_add_tree(_bspl_node * node, _bspl_node * parent, _Bool bit)
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
void _bspl_add_node(lpm_root * root, _bspl_node * node, _bspl_node * parent, _Bool bit)
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
void * _bspl_create()
{
	_bspl_node * node;
	node = malloc(sizeof(_bspl_node));

	if(node == NULL) return errno = FASTNET_OUT_OF_MEMORY, NULL;
	node->left = NULL;
	node->right = NULL;
	node->next = NULL;
	node->type = _BSPL_NODE_PREFIX;
	memset(node->prefix6, 0, 16);

	return node;
}

/**
 * @brief Remove node from hash table
 * @details Does not free memory used by node
 * @param root
 * @param node
 */
void _bspl_remove_htable(lpm_root * root, _bspl_node * node)
{
	assert(root != NULL);
	assert(node != NULL);
	assert(root->htable != NULL);

	unsigned int index = calculate_hash(node->prefix);
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
void _bspl_remove_tree(_bspl_node * parent, _Bool bit)
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
void _bspl_remove_node(lpm_root * root, _bspl_node * node, _bspl_node * parent, _Bool bit)
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
_bspl_node * _bspl_lookup_internal(_bspl_node * node, uint32_t * prefix, uint8_t prefix_len, _bspl_node ** parent, _bspl_node ** other)
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
		*other = *(_bspl_node **) ((void *) node + _BSPL_TREE_OFFSET(!bit));; // *other = bit ? node->left : node->right
		node = *(_bspl_node **) ((void *) node + _BSPL_TREE_OFFSET(bit)); // node = bit ? node->right : node->left
	}
	while(++len != prefix_len && node != NULL);

	return node;
}

lpm_root * _bspl_init(_LPM_RULE default_rule)
{
	lpm_root * root = malloc(sizeof(lpm_root));

	if(root == NULL) return errno = FASTNET_OUT_OF_MEMORY, NULL;
	root->tree = _bspl_create();
	root->tree->type = _BSPL_NODE_PREFIX;
	root->tree->rule = default_rule;
	root->tree->prefix_len = 0;
	root->tree->prefix = 0;

	root->htable = (_bspl_node **) calloc(_BSPL_HTABLE_SIZE, sizeof(_bspl_node *));

	if(root->htable == NULL) return errno = FASTNET_OUT_OF_MEMORY, free(root), NULL;
	_bspl_add_htable(root, root->tree);

	return root;
}

void _bspl_destroy(lpm_root * root)
{
	if(root == NULL) return;
	assert(root->htable != NULL);

	_bspl_node * node, * prev;

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

uint32_t _bspl_get_bits(uint32_t * dst, uint32_t * src, uint8_t length)
{
	unsigned index = (length - 1) / 32;
	memset(dst, 0, 16);
	memcpy(dst, src, index * 4);
	dst[index] = src[index] & (~0 << (32 - (length - abs((length - 1) / 32)*32)));
}


_LPM_RULE _bspl_lookup(lpm_root * root, uint32_t * key, uint8_t length, uint8_t bytes_of_prefix)
{
	assert(root != NULL);
	assert(root->htable != NULL);

	uint32_t prefix_bits[4] = {0}; // extracted part of prefix
	uint8_t prefix_len = length; // binary search actual length
	uint8_t prefix_change = length; // binary search length change
	_bspl_node * node = NULL;

	do
	{
		_bspl_get_bits(prefix_bits, key, prefix_len); // TODO ipv6
		node = root->htable[calculate_hash(prefix_bits[0])];

		while(node != NULL && (node->prefix_len != prefix_len || memcmp(node->prefix6, prefix_bits, bytes_of_prefix) != 0))
		{
			node = node->next;
		}

		prefix_change >>= 1;

		// if(node != NULL){
		// printf("node->prefix %u length %u\n", node->prefix, node->prefix_len);
		// printf("bits->prefix %u length %u\n", prefix_bits, prefix_len);}

		if(node == NULL) prefix_len -= prefix_change;
		else if(node->type == _BSPL_NODE_INTERNAL) prefix_len += prefix_change;
		else break;

	} while(prefix_change > 0);

	if(node == NULL) return root->tree->rule;

	return node->rule;
}

void _bspl_remove(lpm_root * root, uint32_t * prefix, uint8_t prefix_len)
{
	assert(root != NULL);

	_bspl_node * other, * parent;
	_bspl_node * node = _bspl_lookup_internal(root->tree, prefix, prefix_len, &parent, &other);

	// node is leaf node, other node is leaf node and does not contain different prefix from parent node (contructed by leaf-pushing)

	assert(node != NULL);
	assert(other != NULL);

	if(node->type == _BSPL_NODE_PREFIX && other->type == _BSPL_NODE_PREFIX && other->rule == parent->rule)
	{
		assert(parent != NULL);

		_bspl_remove_node(root, node, parent, parent->right == node);
		_bspl_remove_node(root, other, parent, parent->right == other);
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

	_bspl_node * parent, * other;
	_bspl_node * node = _bspl_lookup_internal(root->tree, prefix, prefix_len, &parent, &other);

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

void _bspl_set_prefix(_bspl_node * dst, _bspl_node * src, uint8_t length, _Bool bit)
{
	memcpy(dst->prefix6, src->prefix6, 16);
	dst->prefix6[length/32] |= (bit << (31 - (length - abs((length - 1) / 32)*32)));
}

_Bool _bspl_add(lpm_root * root, uint32_t * prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	assert(root != NULL);
	assert(root->tree != NULL);

	_bspl_node * node = root->tree;
	_bspl_node * parent = root->tree;
	_bspl_node * other;
	_Bool bit;
	uint8_t len = 0;
	uint32_t parent_rule;
	uint32_t prefix_bits;

	do
	{
		assert(parent != NULL);

		bit = prefix[len/32] & 0x80000000; // pick MSB only
		prefix[len/32] <<= 1;
		// prefix_bits = len % 32 == 0 ? 0 : parent->prefix;
		parent_rule = parent->rule;
		node =  bit ? parent->right : parent->left;
		other  =  bit ? parent->left : parent->right;
		parent->type = _BSPL_NODE_INTERNAL;

		if(other == NULL)
		{
			other = _bspl_create();

			if(other == NULL) return errno = FASTNET_OUT_OF_MEMORY, 0;
			_bspl_set_prefix(other, parent, len, !bit);
			// other->prefix = prefix_bits | (!bit << (31 - len));
			other->prefix_len = len + 1;
			other->rule = parent_rule;
			_bspl_add_node(root, other, parent, !bit);

		}

		if(node == NULL)
		{
			node = _bspl_create();

			if(node == NULL) return errno = FASTNET_OUT_OF_MEMORY, 0;
			_bspl_set_prefix(node, parent, len, bit);
			// node->prefix = prefix_bits | (bit << (31 - len));
			node->prefix_len = len + 1;
			node->rule = parent_rule;
			_bspl_add_node(root, node, parent, bit);
		}

		parent = node;
		++len;
	}
	while(prefix_len != len);
	// ip(node->prefix, "add node");
	_bspl_leaf_pushing(node, node->rule, rule);
	// printf("add type %d\n", node->type);
	// printf("add length %d\n", node->prefix_len);
	// printf("prefix_len %d\n", prefix_len);
	// printf("add ptr %p\n", node);

	return 1;
}

/**
 * @brief Construct bspl structures
 * @param default_rule default rule for IPv4
 * @return bspl_root
 */
lpm_root * lpm_init(_LPM_RULE default_rule)
{
	return _bspl_init(default_rule);
}

lpm_root * lpm6_init(_LPM_RULE default_rule)
{
	return _bspl_init(default_rule);
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
 	_bspl_remove(root, (uint32_t *) prefix, prefix_len);
 }

/**
 * @brief Remove node from all bspl structures
 * @param root
 * @param prefix
 * @param prefix_len
 */
void lpm6_remove(lpm_root * root, struct in6_addr * prefix, uint8_t prefix_len)
{
	_bspl_remove(root, (uint32_t *) prefix, prefix_len);
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
	return _bspl_add(root, (uint32_t *) prefix, prefix_len, rule);
}

_Bool lpm6_add(lpm_root * root, struct in6_addr * prefix, uint8_t prefix_len, _LPM_RULE rule)
{
	return _bspl_add(root, (uint32_t *) prefix, prefix_len, rule);
}
