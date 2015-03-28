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
	char * text;
	unsigned length;
	_AC_RULE rule;
} pm_keyword;

typedef struct
{
	_AC_RULE * rule; // nalezene pravidlo
	unsigned size;
	unsigned count;
	unsigned position; // pozice za pravidlem
	char * text; // v jakem textu se hledalo
	unsigned length; // jak je text dlouhy
	_ac_state * state; // v jakem stavu jsem skoncil

} pm_match;

typedef struct
{
	_ac_state * state;
	_ac_queue * queue;
	pm_match * match;
} pm_root;


pm_root * init();
pm_match * match(pm_root * root, char * text, unsigned length);
pm_match * match_next(pm_root * root);
void add(pm_root * root, pm_keyword keywords[], unsigned size);
void destroy(pm_root * root);
void pm_remove(pm_root * root, char * text, unsigned length);
