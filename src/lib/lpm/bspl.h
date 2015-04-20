#include "common.h"

#define _BSPL_HTABLE_SIZE 127
#define _BSPL_TREE_OFFSET(bit) ((bit) * sizeof(struct _bspl_node *))

enum _BSPL_NODE_TYPES
{
	_BSPL_NODE_INTERNAL,
	_BSPL_NODE_PREFIX,
};

uint32_t calculate_hash(uint32_t key);
void _bspl_leaf_pushing(_bspl_node * node, _LPM_RULE rule_original, _LPM_RULE rule_new);
_bspl_node * _bspl_create();
void _bspl_add_htable(lpm_root * root, _bspl_node * node);
void _bspl_add_tree(_bspl_node * node, _bspl_node * parent, _Bool bit);
void _bspl_add(lpm_root * root, _bspl_node * node, _bspl_node * parent, _Bool bit);
void _bspl_remove_htable(lpm_root * root, _bspl_node * node);
void _bspl_remove_tree(_bspl_node * parent, _Bool bit);
void _bspl_remove(lpm_root * root, _bspl_node * node, _bspl_node * parent, _Bool bit);
