#include <stdint.h>
#include <netinet/in.h>
#include "types-precompiled.h"
#include "../common/common.h"

lpm_root * lpm_init(_LPM_RULE default_rule);
_Bool lpm_add(lpm_root * root, struct in_addr * prefix, uint8_t prefix_len, _LPM_RULE rule);
void lpm_update(lpm_root * root, struct in_addr * prefix, uint8_t prefix_len, _LPM_RULE rule);
void lpm_remove(lpm_root * root, struct in_addr * prefix, uint8_t prefix_len);
void lpm_destroy(lpm_root * root);
_LPM_RULE lpm_lookup(lpm_root * root, struct in_addr * key);

lpm6_root * lpm6_init(_LPM_RULE default_rule);
_Bool lpm6_add(lpm6_root * root, struct in6_addr * prefix, uint8_t prefix_len, _LPM_RULE rule);
void lpm6_update(lpm6_root * root, struct in6_addr * prefix, uint8_t prefix_len, _LPM_RULE rule);
void lpm6_remove(lpm6_root * root, struct in6_addr * prefix, uint8_t prefix_len);
void lpm6_destroy(lpm6_root * root);
_LPM_RULE lpm6_lookup(lpm6_root * root, struct in6_addr * key);
