#include "queue.h"

unsigned _increment(queue * root, _Bool start)
{
	unsigned * value = start ? &(root->start) : &(root->end);
	*value = (*value + 1) % root->size;
	return *value;
}

queue * queue_init()
{
	queue * root = malloc(sizeof(queue));

	if(root == NULL) return errno = FASTNET_OUT_OF_MEMORY, NULL;

	root->data = malloc(sizeof(queue_item) * _QUEUE_DEFAULT_SIZE);

	if(root->data == NULL)
	{
		errno = FASTNET_OUT_OF_MEMORY;
		free(root);
		return errno = FASTNET_OUT_OF_MEMORY, NULL;
	}

	root->size = _QUEUE_DEFAULT_SIZE;
	root->start = 0;
	root->end = 0;
	root->empty = 1;

	return root;
}

_Bool queue_empty(queue * root)
{
	assert(root != NULL);

	return root->empty;
}


void queue_destroy(queue * root)
{
	if(root == NULL) return;

	free(root->data);
	free(root);
}

_Bool queue_insert(queue * root, void * pointer, char value_type)
{
	assert(root != NULL);

	queue_item * new_data;

	if(!queue_empty(root) && root->end == root->start)
	{
		new_data = realloc(root->data, sizeof(queue_item) * (root->size + _QUEUE_DEFAULT_SIZE));

		if(new_data == NULL) return errno = FASTNET_OUT_OF_MEMORY, 0;

		root->data = new_data;
		root->size += _QUEUE_DEFAULT_SIZE;

		if(root->start >= root->end)
		{
			unsigned count = root->size - root->start - _QUEUE_DEFAULT_SIZE;
			memmove(&(root->data[root->size - count]), &(root->data[root->start]), sizeof(void *) * count);
			root->start = root->size - count;
		}
	}

	root->empty = 0;
	root->data[root->end].value.pointer = pointer;
	root->data[root->end].type = value_type;
	_increment(root, 0);

	return 1;
}

queue_value queue_front(queue * root)
{
	assert(root != NULL);
	assert(root->data != NULL);
	assert(!queue_empty(root));

	queue_value res = root->data[root->start].value;

	if(root->end == _increment(root, 1)) root->empty = 1;

	return res;
}

stack_value stack_pop(stack * root)
{
	assert(root != NULL);
	assert(!stack_empty(root));

	stack_value value = root->data[--root->end].value;

	if(root->end == root->start) root->empty = 1;

	return value;
}
stack_value stack_top(stack * root)
{
	assert(root != NULL);
	assert(!stack_empty(root));

	return root->data[root->end - 1].value;
}


stack_value stack_top_type(stack * root, char value_type)
{
	assert(root != NULL);
	assert(!stack_empty(root));

	for(int i = root->end - 1; i >= 0; --i)
	{
		if(root->data[i].type == value_type)
		{
			return root->data[i].value;
		}
	}

	assert(0);
}

void stack_free_pointers(stack * root, void (*function) (void *))
{
	assert(root != NULL);
	assert(function != NULL);

	for(unsigned i = 0; i < root->end; ++i)
	{
		if(root->data[i].type == POINTER)
		{
			function(root->data[i].value.pointer);
		}
	}
}

_Bool stack_contains(stack * root, void * pointer, char value_type)
{
	assert(root != NULL);

	for(unsigned i = 0; i < root->end; ++i)
	{
		if(root->data[i].type == value_type && root->data[i].value.pointer == pointer) return 1;
	}

	return 0;
}

void * stack_find(stack * root, stack_value value, _Bool (*match) (stack_value, stack_value))
{
	assert(root != NULL);

	for(unsigned i = 0; i < root->end; ++i)
	{
		if(match(root->data[i].value, value) == 1) return root->data[i].value.pointer;
	}

	return NULL;
}

_Bool stack_push_unique(stack * root, void * pointer, char value_type)
{
	assert(root != NULL);

	return stack_contains(root, pointer, value_type) ? 1 : stack_push(root, pointer, value_type);
}

unsigned stack_size(stack * root)
{
	return root->end;
}
