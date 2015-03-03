#include "lpm.h"

/**
	TODO optimalizace

	TODO ostatní
		ošetřit všechny volání malloc
		rozšířit pro ipv6
		pro střídu 5 (i větší) je potřeba 2^6-1 (63 !!) bitů , což se do int32 nevejde, týká se bitmap
			změnit _TBM_ALL, buildin_popcount × buildin_popcountll
		bude potřeba update jako samostatná fce? nebude to řešit add? nemá šanci zjistit zda už tam přesně tuto adresu s tímto prefixem má
		extend/reduce do jedné fce?
		dá se volat free na null?
		mazat celou větev pokud je zkonstruována pouze pro jeden konkrétní prefix?
**/

#define _TBM_SIZE_INTERNAL (1 << (STRIDE + 1)) - 1
#define _TBM_SIZE_EXTERNAL 1 << STRIDE
#define _TBM_ALL 32

#define GET_STRIDE_BITS(key, position, length) ((key) >> (sizeof(key) * 8 - (position)*STRIDE - (length)) & ~(~0 << (length)))
#define INTERNAL_INDEX(length, value) ((1 << (length)) - 1 + (value))

typedef struct _tbm_node_
{
	// * = MSB
	uint32_t internal;
	// 0000 = MSB
	uint32_t external;

	_LPM_RULE * rule;
	struct _tbm_node_ * child;
} _tbm_node;




uint8_t _tbm_bitsum(uint32_t bitmap, uint8_t bit_position);
uint8_t _tbm_internal_index(uint32_t bit_vector, uint8_t bit_value);
_tbm_node * _tbm_lookup(uint32_t prefix, uint8_t prefix_len, uint8_t * index);
_tbm_node * _tbm_create();
void _tbm_destroy(_tbm_node * node);
void _tbm_extend(_tbm_node * node, uint8_t bit_value, bool shift_child);
void _tbm_reduce(_tbm_node * node, uint8_t bit_value, bool shift_child);
