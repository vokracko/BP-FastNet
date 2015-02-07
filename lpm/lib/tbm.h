#include "lpm.h"

#define STRIDE 2
#define _TBM_SIZE_INTERNAL (1 << (STRIDE + 1)) - 1
#define _TBM_SIZE_EXTERNAL 1 << STRIDE

typedef struct _tbm_node_
{
	unsigned char internal : _TBM_SIZE_INTERNAL;
	unsigned char external : _TBM_SIZE_EXTERNAL;

	_LPM_RULE_SIZE * rule;
	struct _tbm_node * child;
} _tbm_node;


#define get_bits(key, position) ((key >> (sizeof(key)*8 - STRIDE - position*STRIDE)) & (~(~0L) << STRIDE))
