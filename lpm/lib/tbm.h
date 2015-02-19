#include "lpm.h"

#define STRIDE 2
#define _TBM_SIZE_INTERNAL (1 << (STRIDE + 1)) - 1
#define _TBM_SIZE_EXTERNAL 1 << STRIDE
#define _TBM_ALL 33

typedef struct _tbm_node_
{
	// * = MSB
	uint32_t internal;
	// 0000 = MSB
	uint32_t external;

	_LPM_RULE * rule;
	struct _tbm_node_ * child;
} _tbm_node;


#define GET_STRIDE_BITS(key, position) ((key) >> (sizeof(key) * 8 - ((position)+1)*STRIDE ) & ~(~0 << STRIDE))
#define INTERNAL_INDEX(length, value) ((1 << (length)) - 1 + (value))

uint8_t _tbm_bitsum(uint32_t key, uint8_t bit_position);
uint8_t _tbm_internal_index(uint32_t bit_vector, uint8_t bit_value);
_tbm_node * _tbm_lookup(uint32_t prefix, uint8_t prefix_len, uint8_t * index, bool * is_external);
_tbm_node * _tbm_create();
void _tbm_destroy(_tbm_node * node);
void _tbm_extend(_tbm_node * node, uint8_t bit_value, bool shift_child);
void _tbm_reduce(_tbm_node * node, uint8_t bit_value, bool shift_child);
