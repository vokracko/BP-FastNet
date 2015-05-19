#include "common.h"

#define _QUEUE_DEFAULT_SIZE 20
#define stack_init queue_init
#define stack_empty queue_empty
#define stack_destroy queue_destroy

typedef union
{
	void * pointer;
	short number;
} queue_value, stack_value;

typedef struct
{
	queue_value value;
	char type;
} queue_item, stack_item;

typedef struct
{
	unsigned size;
	unsigned start;
	unsigned end;
	_Bool empty;

	queue_item * data;
} queue, stack;


queue *queue_init(void);
_Bool queue_empty(queue *root);
void queue_destroy(queue *root);
_Bool queue_insert(queue * root, void * pointer, char value_type);
queue_value queue_front(queue *root);
stack_value stack_pop(stack * root);
stack_value stack_top(stack *root);
stack_value stack_top_type(stack *root, char value_type);
void stack_free_pointers(stack *root, void (*function)(void *));
_Bool stack_contains(stack * root, void * pointer, char value_type);
_Bool stack_push_unique(stack * root, void * pointer, char value_type);
unsigned stack_size(stack * root);
void * stack_find(stack * root, stack_value value, _Bool (*match) (stack_value, stack_value));
_Bool stack_push(stack * root, void * pointer, char value_type);
