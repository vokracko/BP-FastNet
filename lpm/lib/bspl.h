#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define uint128_t long long
//TODO

typedef struct _bspl_node_
{
	uint8_t type;

	struct _bspl_node_ * next;

	// union
	// {
		// struct
		// {
			struct _bspl_node_ * left;
			struct _bspl_node_ * right;
		// };

		uint32_t rule;
		uint32_t prefix;
		uint8_t prefix_len;
	// };

} _bspl_node;

typedef struct _bspl_node6_
{
	uint8_t type;

	struct _bspl_node6_ * next;

	// union
	// {
	// 	struct
	// 	{
			struct _bspl_node6_ * left;
			struct _bspl_node6_ * right;
		// };

		uint32_t rule;
		uint128_t prefix;
		uint8_t prefix_len;
	// };

} _bspl_node6;

enum _BSPL_NODE_TYPES
{
	_BSPL_NODE_INTERNAL,
	_BSPL_NODE_PREFIX,
	_BSPL_NODE_CREATED,
};

#define _BSPL_HTABLE_SIZE 127

//vrátí prvních N bitů a zbytek nuly
#define get_bits(data, bit_count) (data & (~0L << (32 - bit_count)))
#define get_bit(data, position) ((data >> (position)) & 1)
#define get_byte(data, position) ((data >> (position * 8)) & 0xFF)


