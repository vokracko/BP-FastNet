#define uint128_t long long //TODO vyřešit, použít pole jako u TBM?
#define _LPM_RULE uint8_t

#ifdef bspl // ########################################################

typedef struct _bspl_node
{
	struct _bspl_node * left;
	struct _bspl_node * right;
	struct _bspl_node * next;
	uint8_t type;
	uint32_t prefix;
	uint8_t prefix_len;

	_LPM_RULE rule;

} _bspl_node;

typedef struct
{
	_bspl_node * tree;
	_bspl_node ** htable;

} lpm_root;

#endif

#ifdef tbm // ########################################################
#ifndef STRIDE
#define STRIDE 2
#endif

#define _TBM_SIZE_INTERNAL ((1 << (STRIDE + 1)) - 1) / 32 + 1
#define _TBM_SIZE_EXTERNAL (1 << STRIDE) / 32 + (((1 << STRIDE) % 32) ? 1 : 0)


typedef struct _tbm_node
{
	// * = MSB
	uint32_t internal[_TBM_SIZE_INTERNAL];
	// 0000 = MSB
	uint32_t external[_TBM_SIZE_EXTERNAL];

	_LPM_RULE * rule;
	struct _tbm_node * child;
} _tbm_node;

typedef _tbm_node lpm_root;

#endif
