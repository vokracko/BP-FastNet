#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define _AC_RULE uint8_t

typedef struct _ac_node_
{
	char * key;
	struct _ac_node_ ** next;
	struct _ac_node_ * fallback;
	_AC_RULE rule;
} _ac_node;

void init();
uint8_t match(char * text, _AC_RULE ** matches);
void add(char * text, _AC_RULE rule);
void destroy();
