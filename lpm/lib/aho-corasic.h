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
	struct _ac_node_ * fall;
	_AC_RULE rule;
} _ac_node;
