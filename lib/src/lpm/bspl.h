#include "common.h"

#define _BSPL_HTABLE_SIZE 100000
#define _BSPL_TREE_OFFSET(bit) ((bit) * sizeof(struct _bspl_node *))

enum _BSPL_NODE_TYPES
{
	_BSPL_NODE_INTERNAL,
	_BSPL_NODE_PREFIX
};

void _bspl_add_htable(lpm_root * root, _bspl_node * node);
void _bspl_add_tree(_bspl_node * node, _bspl_node * parent, _Bool bit);
void _bspl_add_node(lpm_root * root, _bspl_node * node, _bspl_node * parent, _Bool bit);
void _bspl_remove_htable(lpm_root * root, _bspl_node * node);
void _bspl_remove_tree(_bspl_node * parent, _Bool bit);
void _bspl_remove_node(lpm_root * root, _bspl_node * node, _bspl_node * parent, _Bool bit);
uint32_t calculate_hash(uint32_t key);
void _bspl_leaf_pushing(_bspl_node * node, _LPM_RULE rule_original, _LPM_RULE rule_new);
lpm_root *_bspl_init(uint8_t default_rule);
void _bspl_destroy(lpm_root *root);
void _bspl_get_bits(uint32_t *dst, uint32_t *src, uint8_t length);
uint8_t _bspl_lookup(lpm_root *root, uint32_t *key, uint8_t length, uint8_t bytes_of_prefix);
void _bspl_remove(lpm_root *root, uint32_t *prefix, uint8_t prefix_len);
void _bspl_update(lpm_root *root, uint32_t *prefix, uint8_t prefix_len, uint8_t rule);
_Bool _bspl_add(lpm_root *root, uint32_t *prefix, uint8_t prefix_len, uint8_t rule);
