#ifdef ALG_dfa

typedef struct _dfa_state
{
	char * key;
	struct _dfa_state ** next;
	unsigned length;
	unsigned char id;
} _dfa_state;

typedef struct _nfa_state
{
	char * key;
	struct _nfa_state ** next;
	unsigned length;
	unsigned char id;

	struct _nfa_state ** next_epsilon;
	unsigned length_epsilon;
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

typedef struct _nfa_state regex_nfa;
typedef struct _dfa_state regex_dfa;

#endif

#ifdef ALG_ddfa

#endif
