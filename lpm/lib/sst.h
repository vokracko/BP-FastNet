#include "lpm.h"

typedef struct _sst_node_
{
	uint32_t shape;
	uint32_t internal;
	uint32_t external;

	_sst_node_ * child;
	_LPM_RULE * rule;

} _sst_node;
