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
	unsigned length;
	unsigned char id;
	unsigned num;
} _dfa_state;

typedef struct _nfa_state
{
	char * key;
	struct _nfa_state ** next;
	unsigned length;
	unsigned char id;

	struct _nfa_state ** next_epsilon;
	unsigned length_epsilon;
	unsigned num;
} _nfa_state;

typedef struct
{
	_nfa_state * start;
	_nfa_state * end;
} _nfa_block;

typedef struct
{
	unsigned length;
	unsigned char id;
	char * input;
} regex_pattern;

typedef struct _dfa_state regex_root;

#endif

#ifdef ddfa

#endif
