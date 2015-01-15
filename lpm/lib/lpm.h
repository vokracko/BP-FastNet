#include <stdint.h>
#define uint128_t long long
//TODO vše i pro ipv6

void lpm_add(uint32_t prefix, uint8_t prefix_len, uint32_t rule);
void lpm_init(uint32_t default_rule, uint128_t default_rule6);
void lpm_destroy();

uint32_t lpm_lookup(uint32_t key);
