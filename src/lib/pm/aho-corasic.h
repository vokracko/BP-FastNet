#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define FAIL -1
#define _AC_RULE uint32_t

typedef struct _ac_state_
{
	char * key;
	struct _ac_state_ ** next;
	struct _ac_state_ * failure;
	_AC_RULE * rule;
	_AC_RULE rule_size;
} _ac_state;

typedef struct _ac_queue_item_
{
	struct _ac_queue_item_ * next;
	_ac_state * state;
} _ac_queue_item;

typedef struct
{
	_ac_queue_item * head;
	_ac_queue_item * tail;
} _ac_queue;

typedef struct
{
	_ac_state * state;
	_ac_queue * queue;
	_AC_RULE * matches;
	unsigned matches_size;
} pm_root;

pm_root * init();
unsigned match(pm_root * root, char * text, _AC_RULE ** matches);
void add(pm_root * root, char * text, _AC_RULE rule);
void destroy(pm_root * root);
