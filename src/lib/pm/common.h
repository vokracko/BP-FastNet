#include "types-precompiled.h"
#include <assert.h>

#define _queue_init list_init
#define _queue_empty list_empty
#define _queue_size list_size
#define _queue_destroy list_destroy
#define _queue_insert list_append_back
#define _queue_pop_front list_pop
#define _queue_front list_first
