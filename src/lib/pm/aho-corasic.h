#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define _AC_RULE uint32_t

typedef struct _ac_node_
{
	char * key;
	struct _ac_node_ ** next;
	struct _ac_node_ * fallback;
	_AC_RULE rule;
} _ac_node;

typedef struct
{
	_ac_node * node;
	// text for which fallback is searched
	char * fallback_buffer;
	unsigned fallback_buffer_size;
	_AC_RULE * matches;
	unsigned matches_size;
} pm_root;

pm_root * init();
unsigned match(pm_root * root, char * text, _AC_RULE ** matches);
void add(pm_root * root, char * text, _AC_RULE rule);
void destroy(pm_root * root);
