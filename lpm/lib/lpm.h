#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


#define uint128_t long long
#define _LPM_RULE_SIZE uint8_t
//TODO vše i pro ipv6

void lpm_init(_LPM_RULE_SIZE default_rule, _LPM_RULE_SIZE default_rule6);
void lpm_add(uint32_t prefix, uint8_t prefix_len, _LPM_RULE_SIZE rule);
void lpm_update(uint32_t prefix, uint8_t prefix_len, _LPM_RULE_SIZE rule);
void lpm_remove(uint32_t prefix, uint8_t prefix_len);
void lpm_destroy();

uint32_t lpm_lookup(uint32_t key);
