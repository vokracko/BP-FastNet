#include "list.h"

_Bool list_empty(list * root)
{
	assert(root != NULL);

	return root->size == 0;
}

list_item * _list_item_create(list_item_value value, char value_type)
{
	list_item * item = malloc(sizeof(list_item));

	if(item == NULL)
	{
		errno = FASTNET_OUT_OF_MEMORY;
		return NULL;
	}

	item->value_type = value_type;
	item->value = value;
	item->next = NULL;

	return item;
}

_Bool list_append_back(list * root, list_item_value value, char value_type)
{
	assert(root != NULL);

	list_item * item = _list_item_create(value, value_type);

	if(item == NULL)
	{
		errno = FASTNET_OUT_OF_MEMORY;
		return 0;
	}

	if(list_empty(root))
	{
		root->head = root->tail = item;
	}
	else
	{
		root->tail->next = item;
		root->tail = item;
	}

	++root->size;

	return 1;
}

_Bool list_append_front(list * root, list_item_value value, char value_type)
{
	assert(root != NULL);

	list_item * item = _list_item_create(value, value_type);

	if(item == NULL)
	{
		errno = FASTNET_OUT_OF_MEMORY;
		return 0;
	}

	if(list_empty(root))
	{
		root->head = root->tail = item;
	}
	else
	{
		item->next = root->head;
		root->head = item;
	}

	++root->size;

	return 1;
}

list_item_value list_pop(list * root)
{
	assert(!list_empty(root));

	list_item * item = root->head;
	list_item_value value = item->value;

	root->head = item->next;

	free(item);

	--root->size;

	return value;
}

unsigned list_size(list * root)
{
	assert(root != NULL);

	return root->size;
}

list * list_init()
{
	list * root = malloc(sizeof(list));

	if(root == NULL)
	{
		errno = FASTNET_OUT_OF_MEMORY;
		return NULL;
	}

	root->head = root->tail = NULL;
	root->size = 0;

	return root;
}

void list_clear(list * root)
{
	while(!list_empty(root))
	{
		list_pop(root);
	}
}

void list_destroy(void * root)
{
	if(root == NULL) return;

	list_clear((list *) root);
	free(root);
}

list_item_value list_front(list * root)
{
	assert(!list_empty(root));

	return root->head->value;
}

list_item_value list_first_type(list * root, char type)
{
	assert(!list_empty(root));

	list_item * item = root->head;

	while(item != NULL)
	{
		if(item->value_type == type) return item->value;
		item = item->next;
	}

	assert(0);
}

void list_free_pointers(list * root, void (*function)(void*))
{
	assert(root != NULL);

	list_item * item = root->head;

	while(item != NULL)
	{
		if(item->value_type == POINTER)
		{
			function(item->value.pointer);
		}

		item = item->next;
	}
}

_Bool list_contains(list * root, list_item_value value, char value_type)
{
	assert(root != NULL);

	list_item * item = root->head;

	while(item != NULL)
	{
		switch(value_type)
		{
			case POINTER:
				if(item->value.pointer == value.pointer) return 1;

			case NUMBER:
				if(item->value.number == value.number) return 1;
		}

		item = item->next;
	}

	return 0;
}

list_item_value * list_find(list * root, list_item_value value, int (*match) (list_item_value, list_item_value))
{
	list_item * item = root->head;

	while(item != NULL)
	{
		if(match(item->value, value)) return &(item->value);

		item = item->next;
	}

	return NULL;
}

_Bool list_append_unique(list * root, list_item_value value, char value_type)
{
	assert(root != NULL);

	list_item * item = root->head;

	while(item != NULL)
	{
		if(item->value.pointer == value.pointer) return 1;

		item = item->next;
	}

	return list_append_back(root, value, value_type);
}