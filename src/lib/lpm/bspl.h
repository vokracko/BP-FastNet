#include "common.h"

/**
	TODO optimalizace
		type a prefix length v jednom byte
		pouze jeden ukazatel na potomky => budou se alokovat najednou při inicializaci -> stejně jsou potřeba pro leaf pushing
		přepsat leaf pushing na nerekurzivní

	TODO ostatní
		ošetřit všechny volání malloc
		rozšířit pro ipv6
		bude potřeba update jako samostatná fce? nebude to řešit add? nemá šanci zjistit zda už tam přesně tuto adresu s tímto prefixem má
		mazat celou větev pokud je zkonstruována pouze pro jeden konkrétní prefix?
**/

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
void _bspl_add_tree(_bspl_node * node, _bspl_node * parent, bool bit);
void _bspl_add(lpm_root * root, _bspl_node * node, _bspl_node * parent, bool bit);
void _bspl_remove_htable(lpm_root * root, _bspl_node * node);
void _bspl_remove_tree(_bspl_node * parent, bool bit);
void _bspl_remove(lpm_root * root, _bspl_node * node, _bspl_node * parent, bool bit);
