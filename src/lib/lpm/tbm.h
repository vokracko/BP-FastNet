#include "common.h"

#define _TBM_ALL_EXTERNAL ((_TBM_SIZE_EXTERNAL) << 5)
#define _TBM_ALL_INTERNAL ((_TBM_SIZE_INTERNAL) << 5)

#define INTERNAL_INDEX(length, value) ((1 << (length)) - 1 + (value))

uint16_t _tbm_bitsum(uint32_t *bitmap, uint16_t bit_position);
void _tbm_zeros(uint32_t *bitmap, uint16_t size);
int32_t _tbm_internal_index(uint32_t *bitmap, uint16_t bit_value);
uint8_t _get_bits(uint32_t *prefix, uint8_t position, uint8_t length);
_tbm_node *_tbm_lookup_internal(lpm_root *root, uint32_t *prefix, uint8_t prefix_len, uint16_t *index);
_Bool _tbm_extend(_tbm_node *node, uint16_t bit_value, _Bool shift_child);
_Bool _tbm_reduce(_tbm_node *node, uint16_t bit_value, _Bool shift_child);
void _tbm_free(_tbm_node *node);
void *_tbm_init(uint8_t default_rule);
_Bool _tbm_add(lpm_root *root, uint32_t *prefix, uint8_t prefix_len, uint8_t rule);
void _tbm_update(lpm6_root *root, uint32_t *prefix, uint8_t prefix_len, uint8_t rule);
void _tbm_remove(lpm_root *root, uint32_t *prefix, uint8_t prefix_len);
void _tbm_destroy(_tbm_node *root);
uint8_t _tbm_lookup(lpm6_root *root, uint32_t *key);
lpm_root *lpm_init(uint8_t default_rule);
lpm6_root *lpm6_init(uint8_t default_rule);
_Bool lpm_add(lpm_root *root, struct in_addr *prefix, uint8_t prefix_len, uint8_t rule);
_Bool lpm6_add(lpm6_root *root, struct in6_addr *prefix, uint8_t prefix_len, uint8_t rule);
void lpm_update(lpm_root *root, struct in_addr *prefix, uint8_t prefix_len, uint8_t rule);
void lpm6_update(lpm6_root *root, struct in6_addr *prefix, uint8_t prefix_len, uint8_t rule);
void lpm_remove(lpm_root *root, struct in_addr *prefix, uint8_t prefix_len);
void lpm6_remove(lpm6_root *root, struct in6_addr *prefix, uint8_t prefix_len);
void lpm_destroy(lpm_root *root);
void lpm6_destroy(lpm6_root *root);
uint8_t lpm_lookup(lpm_root *root, struct in_addr *key);
uint8_t lpm6_lookup(lpm6_root *root, struct in6_addr *key);
