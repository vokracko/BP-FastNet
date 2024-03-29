#include <pthread.h>
#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "types-precompiled.h"
#include "../common/common.h"

#define GET_BIT_LSB(data, position) ((data >> (position)) & 1)
#define GET_BIT_MSB(data, position) (((data)[(position) / 32] >> (31 - ((position) % 32))) & 1)

#define CLEAR_BIT_LSB(data, position) (data &= ~(1 << (position)))
#define CLEAR_BIT_MSB(data, position) (data &= ~(1 << (sizeof(data)*8-1 - (position))))

#define SET_BIT_LSB(data, position) (data |= (1 << (position)))
#define SET_BIT_MSB(data, position) (data |= (1 << (sizeof(data)*8-1 - (position))))

#define GET_BITS_LSB(data, bit_count) (data & ~(~0L << (bit_count)))
#define GET_BITS_MSB(data, bit_count) ((data) & (((uint32_t) ~0) << (sizeof((data))*8 - (bit_count))))

#define GET_BYTE_LSB(data, position) ((data >> ((position) * 8)) & 0xFF)
#define GET_BYTE_MSB(data, position) ((data >> ((sizeof(data) - (position) - 1) * 8)) & 0xFF)
