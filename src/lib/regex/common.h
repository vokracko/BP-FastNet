#include "types-precompiled.h"
#include "../general/list.h"
#include <stdlib.h>
#include <assert.h>

#define _stack_init list_init
#define _stack_empty list_empty
#define _stack_size list_size
#define _stack_destroy list_destroy
#define _stack_push list_append_front
#define _stack_pop list_pop
#define _stack_top list_first
