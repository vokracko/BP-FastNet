#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define FAIL -1
#define NONE 0
#define _AC_RULE uint32_t

typedef struct _ac_state_
{
	char * key;
	struct _ac_state_ ** next;
	struct _ac_state_ * failure;
	uint16_t path_count;
	_AC_RULE rule;
	_AC_RULE * additional_rule;
	_AC_RULE additional_size;
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

typedef struct
{
	char * text;
	unsigned length;
	_AC_RULE rule;
} pm_pair;

pm_root * init();
unsigned match(pm_root * root, char * text, unsigned length, _AC_RULE ** matches);
void add(pm_root * root, pm_pair keywords[], unsigned size);
void destroy(pm_root * root);
void pm_remove(pm_root * root, char * text, unsigned length);
