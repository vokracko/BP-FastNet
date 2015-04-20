#include "common.h"

#define _QUEUE_DEFAULT_SIZE 20
typedef struct
{
	unsigned size;
	unsigned start;
	unsigned end;
	void ** data;
	_Bool empty;
} queue;


queue * queue_init();
_Bool queue_empty(queue * root);
void queue_destroy(queue * root);
_Bool queue_insert(queue * root, void * value);
void * queue_front(queue * root);
