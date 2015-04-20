#include "common.h"

#define _TBM_ALL_EXTERNAL ((_TBM_SIZE_EXTERNAL) << 5)
#define _TBM_ALL_INTERNAL ((_TBM_SIZE_INTERNAL) << 5)

#define GET_STRIDE_BITS(key, position, length) ((key) >> (sizeof(key) * 8 - (position)*STRIDE - (length)) & ~(~0 << (length)))
#define INTERNAL_INDEX(length, value) ((1 << (length)) - 1 + (value))

void _tbm_zeros(uint32_t * bitmap, uint16_t size);
uint16_t _tbm_bitsum(uint32_t * bitmap, uint16_t bit_position);
int32_t _tbm_internal_index(uint32_t * bit_vector, uint16_t bit_value);
_tbm_node * _tbm_lookup(lpm_root * root, uint32_t prefix, uint8_t prefix_len, uint16_t * index);
_tbm_node * _tbm_create();
void _tbm_destroy(_tbm_node * node);
_Bool _tbm_extend(_tbm_node * node, uint16_t bit_value, _Bool shift_child);
_Bool _tbm_reduce(_tbm_node * node, uint16_t bit_value, _Bool shift_child);
