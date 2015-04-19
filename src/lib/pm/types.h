#define PM_RULE uint32_t
#define PM_RULE_NONE 0

#ifndef PM_RULE_NONE
#error PM_RULE_NONE not defined
#endif


#ifdef ALG_ac

#include "../general/list.h"

typedef struct _ac_state_
{
	char * key;
	struct _ac_state_ ** next;
	struct _ac_state_ * failure;
	uint16_t path_count;
	PM_RULE rule;
	PM_RULE * additional_rule;
	PM_RULE additional_size;
} _ac_state;

typedef struct
{
	char * content;
	unsigned length;
	PM_RULE rule;
} pm_keyword;

typedef struct
{
	PM_RULE * rule; // matched rules
	unsigned size; // size of rule array
	unsigned count; // number of matches in rule array
	unsigned position; // passition after matched rule
	char * input; // match patterns in this text
	unsigned length; // length of text
	_ac_state * state; // last state
} pm_result;

typedef struct
{
	_ac_state * state;
	list * queue;
	pm_result * result;
} pm_root;

#endif
