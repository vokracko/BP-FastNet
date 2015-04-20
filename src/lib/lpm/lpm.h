#include <stdint.h>
#include <stdbool.h>
#include "types-precompiled.h"

lpm_root * lpm_init(_LPM_RULE default_rule);
_Bool lpm_add(lpm_root * root, uint32_t prefix, uint8_t prefix_len, _LPM_RULE rule);
void lpm_update(lpm_root * root, uint32_t prefix, uint8_t prefix_len, _LPM_RULE rule);
void lpm_remove(lpm_root * root, uint32_t prefix, uint8_t prefix_len);
_Bool lpm_destroy(lpm_root * root);
uint32_t lpm_lookup(lpm_root * root, uint32_t key);

