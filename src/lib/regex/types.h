#define PM_RULE uint32_t
#define PM_RULE_NONE 0

#ifndef PM_RULE_NONE
#error PM_RULE_NONE not defined
#endif


#ifdef dfa

typedef struct _dfa_state
{
	char * key;
	struct _dfa_state ** next;
	struct _dfa_state ** next_epsilon;
	unsigned length;
	unsigned length_epsilon;
	unsigned char id;
} _dfa_state;

typedef struct
{
	_dfa_state * start;
	_dfa_state * end;
} _dfa_block;

typedef struct
{
	unsigned length;
	unsigned char id;
	char * input;
} regex_pattern;

typedef struct
{
	unsigned count;
	_dfa_state ** patterns;
} regex_root;

#endif

#ifdef ddfa

#endif
