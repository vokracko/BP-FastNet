#include "common.h"

typedef union
{
	void * pointer;
	short number;
} list_item_value;

typedef struct list_item
{
	struct list_item * next;
	char value_type;
	list_item_value value;
} list_item;

// typedef list_item stack_item;
typedef list_item queue_item;

typedef struct
{
	list_item * head;
	list_item * tail;
	unsigned size;
} list;

typedef list queue;
// typedef list stack;

typedef list_item_value queue_item_value;
// typedef list_item_value stack_item_value;

unsigned list_size(list * root);
void list_destroy(void * root);
list * list_init();
_Bool list_empty(list * root);
_Bool list_append_front(list * root, list_item_value value, char value_type);
_Bool list_append_back(list * root, list_item_value value, char value_type);
list_item_value list_pop(list * root);
list_item_value list_front(list * root);
list_item_value list_first_type(list * root, char type);
void list_free_pointers(list * root, void (*function)(void*));
_Bool list_contains(list * root, list_item_value, char value_type);
void list_clear(list * root);
list_item_value * list_find(list * root, list_item_value value, int (*match) (list_item_value, list_item_value));
_Bool list_append_unique(list * root, list_item_value value, char value_type);
