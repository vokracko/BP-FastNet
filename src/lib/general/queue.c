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

	if(root == NULL) return NULL;

	root->data = malloc(sizeof(void *) * _QUEUE_DEFAULT_SIZE);

	if(root->data == NULL)
	{
		errno = _OUT_OF_MEMORY;
		free(root);
		return NULL;
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

_Bool queue_insert(queue * root, void * value)
{
	assert(root != NULL);

	void ** new_data;

	if(!queue_empty(root) && root->end == root->start)
	{
		new_data = realloc(root->data, sizeof(void *) * (root->size + _QUEUE_DEFAULT_SIZE));

		if(new_data == NULL)
		{
			errno = _OUT_OF_MEMORY;
			return 0;
		}

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
	root->data[root->end] = value;
	_increment(root, 0);

	return 1;
}

void * queue_front(queue * root)
{
	assert(root != NULL);
	assert(root->data != NULL);
	assert(!queue_empty(root));

	void * res = root->data[root->start];

	if(root->end == _increment(root, 1)) root->empty = 1;

	return res;
}
