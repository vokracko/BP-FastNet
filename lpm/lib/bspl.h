#include "lpm.h"

/**
	TODO optimalizace
		type a prefix length v jednom byte
		pouze jeden ukazatel na potomky => budou se alokovat najednou při inicializaci -> stejně jsou potřeba pro leaf pushing
		přepsat leaf pushing na nerekurzivní

	TODO ostatní
		ošetřit všechny volání malloc
		rozšířit pro ipv6
**/

#define _BSPL_HTABLE_SIZE 127
#define _BSPL_TREE_OFFSET(bit) ((bit) * sizeof(struct _bspl_node *))



typedef struct _bspl_node_
{
	struct _bspl_node_ * left;
	struct _bspl_node_ * right;
	struct _bspl_node_ * next;
	uint8_t type;
	uint32_t prefix;
	uint8_t prefix_len;

	_LPM_RULE rule;

} _bspl_node;

typedef struct _bspl_node6_
{
	uint8_t type;

	struct _bspl_node6_ * next;

	// union
	// {
	// 	struct
	// 	{
			struct _bspl_node6_ * left;
			struct _bspl_node6_ * right;
		// };

		_LPM_RULE rule;
		uint128_t prefix;
		uint8_t prefix_len;
	// };

} _bspl_node6;

enum _BSPL_NODE_TYPES
{
	_BSPL_NODE_INTERNAL,
	_BSPL_NODE_PREFIX,
};

uint32_t calculate_hash(uint32_t key);
void _bspl_leaf_pushing(_bspl_node * node, _LPM_RULE rule_original, _LPM_RULE rule_new);
_bspl_node * _bspl_create();
void _bspl_add_htable(_bspl_node * node);
void _bspl_add_tree(_bspl_node * node, _bspl_node * parent, bool bit);
void _bspl_add(_bspl_node * node, _bspl_node * parent, bool bit);
void _bspl_remove_htable(_bspl_node * node);
void _bspl_remove_tree(_bspl_node * parent, bool bit);
void _bspl_remove(_bspl_node * node, _bspl_node * parent, bool bit);
