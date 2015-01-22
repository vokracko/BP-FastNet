#include <stdint.h>
#define uint128_t long long
#define _BSPL_RULE uint8_t
//TODO vše i pro ipv6

void lpm_init(_BSPL_RULE default_rule, _BSPL_RULE default_rule6);
void lpm_add(uint32_t prefix, uint8_t prefix_len, _BSPL_RULE rule);
void lpm_update(uint32_t prefix, uint8_t prefix_len, _BSPL_RULE rule);
void lpm_remove(uint32_t prefix, uint8_t prefix_len);
void lpm_destroy();

uint32_t lpm_lookup(uint32_t key);
