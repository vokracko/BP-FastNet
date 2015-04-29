#include "stack.h"

stack * stack_init()
{
	stack * root = malloc(sizeof(stack));

	if(root == NULL) return NULL;

	root->data = malloc(sizeof(stack_item) * _STACK_DEFAULT_SIZE);

	if(root->data == NULL)
	{
		errno = FASTNET_OUT_OF_MEMORY;
		free(root);
		return NULL;
	}

	root->size = _STACK_DEFAULT_SIZE;
	root->top = -1;

	return root;
}

_Bool stack_empty(stack * root)
{
	assert(root != NULL);

	return root->top == -1;
}

int stack_size(stack * root)
{
	assert(root != NULL);

	return root->top + 1;
}

void stack_destroy(stack * root)
{
	if(root == NULL) return;

	free(root->data);
	free(root);
}

_Bool stack_push(stack * root, stack_item_value value, char value_type)
{
	assert(root != NULL);

	stack_item * new_data;

	if(root->size == root->top + 1)
	{
		new_data = realloc(root->data, sizeof(stack_item) * (root->size + _STACK_DEFAULT_SIZE));

		if(new_data == NULL)
		{
			errno = FASTNET_OUT_OF_MEMORY;
			return 0;
		}

		root->size += _STACK_DEFAULT_SIZE;
		root->data = new_data;
	}

	++(root->top);
	root->data[root->top].value = value;
	root->data[root->top].type = value_type;

	return 1;
}

_Bool stack_push_unique(stack * root, stack_item_value value, char value_type)
{
	assert(root != NULL);

	return stack_contains(root, value, value_type) ? 1 : stack_push(root, value, value_type);
}

stack_item_value stack_pop(stack * root)
{
	assert(root != NULL);
	assert(root->data != NULL);
	assert(!stack_empty(root));

	return root->data[root->top--].value;
}

stack_item_value stack_top(stack * root)
{
	assert(root != NULL);
	assert(root->data != NULL);

	return root->data[root->top].value;
}

stack_item_value stack_top_type(stack * root, char value_type)
{
	assert(root != NULL);
	assert(root->data != NULL);

	for(int i = root->top; i >= 0; --i)
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
	assert(root->data != NULL);
	assert(function != NULL);

	for(int i = 0; i <= root->top; ++i)
	{
		if(root->data[i].type == POINTER)
		{
			function(root->data[i].value.pointer);
		}
	}
}

_Bool stack_contains(stack * root, stack_item_value value, char value_type)
{
	assert(root != NULL);

	if(root->top == -1) return 0;

	for(int i = 0; i <= root->top; ++i)
	{
		if(root->data[i].type == value_type && root->data[i].value.pointer == value.pointer) return 1;
	}

	return 0;
}

stack_item_value * stack_find(stack * root, stack_item_value value, int (*match) (stack_item_value, stack_item_value))
{
	assert(root != NULL);
	assert(root->data != NULL);

	for(int i = 0; i <= root->top; ++i)
	{
		if(match(root->data[i].value, value) == 1) return &(root->data[i].value);
	}

	return NULL;
}

void stack_clear(stack * root)
{
	assert(root != NULL);
	assert(root->data != NULL);

	root->top = -1;
}
