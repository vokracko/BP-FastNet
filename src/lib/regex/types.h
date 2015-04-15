#define PM_RULE uint32_t
#define PM_RULE_NONE 0

#ifndef PM_RULE_NONE
#error PM_RULE_NONE not defined
#endif


#ifdef dfa

typedef struct _state
{
	char * key;
	struct _state ** next;
	struct _state ** next_epsilon;
	unsigned length;
	unsigned length_epsilon;
	unsigned char end_state;
} _state;

typedef struct
{
	_state * start;
	_state * end;
} _construction_block;

typedef _state regex_pattern;

#endif

#ifdef ddfa

#endif
