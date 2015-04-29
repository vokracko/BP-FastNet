#pragma once

#include "common.h"

#define _STACK_DEFAULT_SIZE 20

typedef union
{
	short number;
	void * pointer;
} stack_item_value;

typedef struct
{
	char type;
	stack_item_value value;
} stack_item;

typedef struct
{
	stack_item * data;
	int size;
	int top;
} stack;

stack_item_value * stack_find(stack * root, stack_item_value value, int (*match) (stack_item_value, stack_item_value));
_Bool stack_contains(stack * root, stack_item_value value, char value_type);
void stack_free_pointers(stack * root, void (*function) (void *));
stack_item_value stack_top_type(stack * root, char value_type);
stack_item_value stack_top(stack * root);
stack_item_value stack_pop(stack * root);
_Bool stack_push_unique(stack * root, stack_item_value value, char value_type);
_Bool stack_push(stack * root, stack_item_value value, char value_type);
stack * stack_init();
_Bool stack_empty(stack * root);
int stack_size(stack * root);
void stack_destroy(stack * root);
void stack_clear(stack * root);
