#include "../common/queue.h"
#include "common.h"
#include <strings.h>
#include <limits.h>

enum SYMBOLS
{
	QUESTION_MARK = 300,
	STAR,
	PLUS, // 302
	OPEN_PAREN,
	CLOSE_PAREN, // 304
	OPEN_BRACKET,
	CLOSE_BRACKET, // 306
	OR,
	SYMBOL, // 308
	STACK_BOTTOM,
	END, // 310
	CONCAT,
	START, // 312
	DOT
};

enum
{
	FAIL,
	SHIFT,
	REDUCE,
	EQUAL,
	FIN,
	OK
};

#define EPSILON 400
#define ID_NONE 0
#define NOT_MATCH -1
#define _DFA_CLEAR(free_dfa_states) _dfa_clear(root, nfa_list, move_states, pairs_created, closure_states, pairs_unprocessed, free_dfa_states)

short validation_table[11][13] =
{
	{FAIL,FAIL,FAIL,FAIL,OK,FAIL,OK,FAIL,OK,FAIL,FAIL,FAIL,FAIL},
	{FAIL,FAIL,FAIL,FAIL,OK,FAIL,OK,FAIL,OK,FAIL,FAIL,FAIL,FAIL},
	{FAIL,FAIL,FAIL,FAIL,OK,FAIL,OK,FAIL,OK,FAIL,FAIL,FAIL,FAIL},
	{CONCAT,CONCAT,CONCAT,OK,CONCAT,FAIL,CONCAT,OK,CONCAT,OK,FAIL,OK,OK},
	{OK,OK,OK,FAIL,OK,FAIL,OK,FAIL,OK,FAIL,FAIL,FAIL,FAIL},
	{CONCAT,CONCAT,CONCAT,OK,CONCAT,FAIL,CONCAT,OK,CONCAT,OK,FAIL,OK,OK},
	{FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL},
	{OK,OK,OK,FAIL,OK,FAIL,OK,OK,OK,FAIL,FAIL,FAIL,FAIL},
	{CONCAT,CONCAT,CONCAT,OK,CONCAT,OK,CONCAT,OK,CONCAT,OK,FAIL,OK,OK},
	{FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL},
	{OK,OK,OK,FAIL,OK,FAIL,OK,FAIL,OK,FAIL,FAIL,FAIL,FAIL}
};

char precedence_table[12][12] =
{
	{FAIL,FAIL,FAIL,SHIFT,FAIL,FAIL,FAIL,SHIFT,FAIL,SHIFT,FAIL,SHIFT},
	{FAIL,FAIL,FAIL,SHIFT,FAIL,FAIL,FAIL,SHIFT,FAIL,SHIFT,FAIL,SHIFT},
	{FAIL,FAIL,FAIL,SHIFT,FAIL,FAIL,FAIL,SHIFT,FAIL,SHIFT,FAIL,SHIFT},
	{FAIL,FAIL,FAIL,SHIFT,FAIL,FAIL,FAIL,SHIFT,FAIL,SHIFT,FAIL,SHIFT},
	{REDUCE,REDUCE,REDUCE,EQUAL,FAIL,FAIL,FAIL,REDUCE,FAIL,FAIL,FAIL,REDUCE},
	{FAIL,FAIL,FAIL,SHIFT,FAIL,FAIL,FAIL,SHIFT,FAIL,SHIFT,FAIL,SHIFT},
	{FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL},
	{REDUCE,REDUCE,REDUCE,SHIFT,FAIL,FAIL,FAIL,REDUCE,FAIL,SHIFT,FAIL,REDUCE},
	{FAIL,FAIL,FAIL,SHIFT,FAIL,FAIL,FAIL,SHIFT,FAIL,SHIFT,FAIL,SHIFT},
	{FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL},
	{REDUCE,REDUCE,REDUCE,FAIL,FAIL,FAIL,FAIL,REDUCE,FAIL,FIN,FAIL,REDUCE},
	{REDUCE,REDUCE,REDUCE,SHIFT,FAIL,FAIL,FAIL,SHIFT,FAIL,SHIFT,FAIL,REDUCE}
};

typedef struct
{
	_dfa_state * dfa_state;
	stack * nfa_states;
} _dfa_pair;

void *_out_of_memory(void);
char _escape(char *input, unsigned length, unsigned *position);
short _get_symbol(char *input, unsigned length, unsigned *position);
_Bool _is_char(short symbol);
_Bool _add_transition(char **key, void ***next, unsigned *length, short symbol, void *next_pointer);
_Bool _nfa_add_transition(_nfa_state *state, short symbol, _nfa_state *next);
_Bool _dfa_add_transition(_dfa_state *state, short symbol, _dfa_state *next);
_Bool _nfa_add_epsilon_transition(_nfa_state *state, _nfa_state *next);
_nfa_state *_nfa_create_state(void);
_dfa_state *_dfa_create_state(void);
void _nfa_state_free(_nfa_state *state);
void _dfa_state_free(_dfa_state *state);
void _nfa_state_destroy(stack *already_freed, _nfa_state *state);
void _dfa_state_destroy(stack *already_freed, _dfa_state *state);
void _dfa_destroy(_dfa_state *state);
void _nfa_destroy(_nfa_state *state);
_nfa_block *_nfa_create_block(short symbol);
_nfa_block *_nfa_create_dot_block(void);
_dfa_pair *_dfa_create_pair(stack **nfa_states);
void _nfa_block_destroy(void *block);
void _dfa_pair_free(void *pair);
void _dfa_pair_destroy(void *pair);
void *_nfa_clear(stack *stack_);
void *_dfa_clear(_nfa_state *root, stack *nfa_states, stack *move_states, stack *pairs_created, stack *closure_states, stack *pairs_unprocessed, _Bool free_dfa_states);
_Bool _concat(stack *stack_);
_Bool _or(stack *stack_);
_Bool _quantificator(stack *stack_, short operation);
_nfa_block *_enumeration(char *input, unsigned length, unsigned *position);
short _validate(short current, short prev);
unsigned _precedence(stack *stack_, short current);
_Bool _reduce(stack *stack_);
_Bool _dfa_epsilon_closure(stack *nfa, stack *closure);
_Bool _dfa_move(stack *input_states, stack *output_states, char character);
_Bool _dfa_match_processed(stack_value first, stack_value second);
regex_nfa *_nfa_construct(regex_pattern pattern);
regex_dfa *_nfa_convert(regex_nfa *root);
regex_nfa *regex_construct_nfa(regex_pattern *patterns, unsigned count);
regex_dfa *regex_construct_dfa(regex_pattern *patterns, unsigned count);
void regex_destroy_nfa(regex_nfa *root);
void regex_destroy_dfa(regex_dfa *root);
int regex_match_nfa(regex_nfa *regex, char *input, unsigned length);
int regex_match_dfa(regex_dfa *regex, char *input, unsigned length);
