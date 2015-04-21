#define uint128_t long long
#define _LPM_RULE uint8_t

#ifdef ALG_bspl // ########################################################

typedef struct _bspl_node_common_
{
	struct _bspl_node_common_ * left;
	struct _bspl_node_common_ * right;
	struct _bspl_node_common_ * next;
	uint8_t type;
	_LPM_RULE rule;
	uint8_t prefix_len;
} _bspl_node_common;

typedef struct _bspl_node
{
	_bspl_node_common common;
	uint32_t prefix;

} _bspl_node;

typedef struct _bspl_node6
{
	_bspl_node_common common;
	struct in6_addr prefix;
} _bspl_node6;

typedef struct
{
	_bspl_node_common * tree;
	_bspl_node_common ** htable;

} lpm_root;

typedef lpm_root lpm6_root;

#endif

#ifdef ALG_tbm // ########################################################
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
typedef _tbm_node lpm6_root;

#endif
