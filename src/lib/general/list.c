#include "list.h"
#include <assert.h>
#include <stdlib.h>

_Bool list_empty(list * root)
{
	assert(root != NULL);

	return root->head == NULL;
}

void list_append_back(list * root, list_item_value value)
{
	assert(root != NULL);

	list_item * item = malloc(sizeof(list_item));

	item->value = value;
	item->next = NULL;

	if(list_empty(root))
	{
		root->head = root->tail = item;
	}
	else
	{
		root->tail->next = item;
		root->tail = item;
	}
}

void list_append_front(list * root, list_item_value value)
{
	assert(root != NULL);

	list_item * item = malloc(sizeof(list_item));

	item->value = value;
	item->next = NULL;

	if(list_empty(root))
	{
		root->head = root->tail = item;
	}
	else
	{
		item->next = root->head;
		root->head = item;
	}
}

list_item_value list_pop(list * root)
{
	assert(root != NULL);

	list_item * item = root->head;
	list_item_value value = item->value;

	root->head = item->next;

	free(item);

	return value;
}
