#define PM_RULE uint32_t
#define PM_RULE_NONE 0

#ifndef PM_RULE_NONE
#error PM_RULE_NONE not defined
#endif


#ifdef ALG_ac

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
	PM_RULE * rule;
	unsigned size;
	unsigned count;
	unsigned position;
	char * input;
	unsigned length;
	_ac_state * state;
} pm_result;

typedef _ac_state pm_root;

#endif
