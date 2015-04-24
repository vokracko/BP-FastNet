#include "common.h"

#define _BSPL_HTABLE_SIZE 100000
#define _BSPL_TREE_OFFSET(bit) ((bit) * sizeof(struct _bspl_node *))

enum _BSPL_NODE_TYPES
{
	_BSPL_NODE_INTERNAL,
	_BSPL_NODE_PREFIX,
};
