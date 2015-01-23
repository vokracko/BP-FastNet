#include "lpm.h"
//TODO
/** TODO optimalizace
	type a prefix length v jednom byte
	pouze jeden ukazatel na potomky => budou se alokovat najednou při inicializaci ?? bude to zabírat více místa
**/


typedef struct _bspl_node_
{
	struct _bspl_node_ * left;
	struct _bspl_node_ * right;
	struct _bspl_node_ * next;
	uint8_t type;
	uint32_t prefix;
	uint8_t prefix_len;

	_LPM_RULE_SIZE rule;

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

		_LPM_RULE_SIZE rule;
		uint128_t prefix;
		uint8_t prefix_len;
	// };

} _bspl_node6;

enum _BSPL_NODE_TYPES
{
	_BSPL_NODE_INTERNAL,
	_BSPL_NODE_PREFIX,
};

#define _BSPL_HTABLE_SIZE 127
#define _BSPL_TREE_OFFSET(bit) (bit * sizeof(struct _bspl_node *))

//vrátí prvních N bitů a zbytek nuly
#define get_bits(data, bit_count) (data & (~0L << (32 - bit_count)))
#define get_bit(data, position) ((data >> (position)) & 1)
#define get_byte(data, position) ((data >> (position * 8)) & 0xFF)


