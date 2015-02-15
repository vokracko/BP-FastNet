#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


//TODO bude potřeba update jako samostatná fce? nebude to řešit add? nemá šanci zjistit zda už tam přesně tuto adresu s tímto prefixem má
//TODO malloc NULL

#define uint128_t long long
#define _LPM_RULE uint8_t
//TODO vše i pro ipv6

void lpm_init(_LPM_RULE default_rule, _LPM_RULE default_rule6);
void lpm_add(uint32_t prefix, uint8_t prefix_len, _LPM_RULE rule);
void lpm_update(uint32_t prefix, uint8_t prefix_len, _LPM_RULE rule);
void lpm_remove(uint32_t prefix, uint8_t prefix_len);
void lpm_destroy();

uint32_t lpm_lookup(uint32_t key);

#define GET_BIT_LSB(data, position) ((data >> (position)) & 1)
#define GET_BIT_MSB(data, position) ((data >> (31 - (position))) & 1)

#define CLEAR_BIT_LSB(data, position) (data &= ~(1 << (position)))
#define CLEAR_BIT_MSB(data, position) (data &= ~(1 << (sizeof(data)*8-1 - (position))))

#define SET_BIT_LSB(data, position) (data |= (1 << (position)))
#define SET_BIT_MSB(data, position) (data |= (1 << (sizeof(data)*8-1 - (position))))

#define GET_BITS_LSB(data, bit_count) (data & ~(~0L << (bit_count)))
#define GET_BITS_MSB(data, bit_count) (data & (~0L << (sizeof(data)*8 - (bit_count))))

#define GET_BYTE_LSB(data, position) ((data >> ((position) * 8)) & 0xFF)
#define GET_BYTE_MSB(data, position) ((data >> ((sizeof(data) - (position) - 1) * 8)) & 0xFF)
