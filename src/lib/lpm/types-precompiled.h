# 1 "types.h"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "types.h"
#define STRIDE 2
#define tbm 1
#undef bspl
# 37 "types.h"
typedef struct _tbm_node
{

 uint32_t internal[((1 << (2 + 1)) - 1) / 32 + 1];
#define _TBM_SIZE_INTERNAL ((1 << (STRIDE + 1)) - 1) / 32 + 1
 uint32_t external[(1 << 2) / 32 + (((1 << 2) % 32) ? 1 : 0)];
#define _TBM_SIZE_EXTERNAL (1 << STRIDE) / 32 + (((1 << STRIDE) % 32) ? 1 : 0)
 uint8_t * rule;
#define _LPM_RULE uint8_t
# 45 "types.h"
 struct _tbm_node * child;
} _tbm_node;

typedef _tbm_node lpm_root;
