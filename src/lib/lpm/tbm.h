#include "common.h"

#define _TBM_ALL_EXTERNAL ((_TBM_SIZE_EXTERNAL) << 5)
#define _TBM_ALL_INTERNAL ((_TBM_SIZE_INTERNAL) << 5)

#define GET_STRIDE_BITS(key, position, length) ((key) >> (sizeof(key) * 8 - (position) - (length)) & ~(~0 << (length)))
#define INTERNAL_INDEX(length, value) ((1 << (length)) - 1 + (value))

void _tbm_zeros(uint32_t * bitmap, uint16_t size);
uint16_t _tbm_bitsum(uint32_t * bitmap, uint16_t bit_position);
int32_t _tbm_internal_index(uint32_t * bit_vector, uint16_t bit_value);
_tbm_node * _tbm_lookup(lpm_root * root, uint32_t * prefix, uint8_t prefix_len, uint16_t * index, _Bool ipv6);
_tbm_node * _tbm_create();
void _tbm_destroy(_tbm_node * node);
_Bool _tbm_extend(_tbm_node * node, uint16_t bit_value, _Bool shift_child);
_Bool _tbm_reduce(_tbm_node * node, uint16_t bit_value, _Bool shift_child);


/**
 * key - uint32_t[4]
 * position - from which part of address to extract <STRIDE> bits
 * length - most of the time same as <STRIDE> but sometimes it is smaller
 *
 * <STRIDE> can be overlaping bytes thus extraction of uint16_t
 *
 * (((uint16_t) ((uint8_t *) key)[(position / 3)]) = get uint16_t which contains requested bits
 * ((position)/8) * 8 = strip index to multipes of 8
 * + 16 = end of uint16_t
 */

#define GET_STRIDE_BITS_IPV6(key, position, length) \
	((htons(((uint16_t *) ((uint8_t *)key + (position / 8)))[0]) >> \
	(((position)/8) * 8 + 16 - (position) - STRIDE + STRIDE - (length))) & ~(~0 << (length)))
