#include "dfa.h"

char escape(char * input, unsigned length, unsigned * position)
{
	char character;

	if(length == *position + 1) return '\\';

	switch(input[*position + 1])
	{
		case '[': ++*position;character =  '[';
		case ']': ++*position;character =  ']';
		case '(': ++*position;character =  '(';
		case ')': ++*position;character =  ')';
		case '+': ++*position;character =  '+';
		case '?': ++*position;character =  '?';
		case '*': ++*position;character =  '*';
		case '|': ++*position;character =  '|';
		case '.': ++*position;character =  '.';
		default: character =  '\\';
	}

	return character;
}

_state * _create_state()
{
	_state * state = malloc(sizeof(_state));
	state->key = NULL;
	state->next = NULL;
	state->length = 0;

	return state;
}

void _add_transition(_state * state, short symbol, _state * next)
{
	++state->length;
	state->key = realloc(state->key, state->length);
	state->next = realloc(state->next, state->length);
	state->key[state->length - 1] = symbol;
	state->next[state->length - 1] = next;
}

_construction_block * _create_block(short symbol)
{
	_construction_block * block = malloc(sizeof(_construction_block));
	block->start = _create_state();
	block->end = _create_state();

	_add_transition(block->start, symbol, block->end);

	return block;
}



/**
 * @brief Concatenate two construction blocks
 * @param stack_ for construction blocks
 */
void operation_concatenation(stack * stack_)
{
	assert(stack_ != NULL);
	assert(_stack_size(stack_) >= 2);

	stack_item_value value;
	_construction_block * first;
	_construction_block * second;

	value = _stack_pop(stack_);
	second = value.pointer;

	value = _stack_pop(stack_);
	first = value.pointer;

	_add_transition(first->end, EPSILON, second->start);
	first->end = second->start;

	free(second);

	value.pointer = first;
	_stack_push(stack_, value, POINTER);
}

void operation_or_internal(_construction_block * first, _construction_block * second)
{
	_state * epsilon = _create_state();
	// create end state
	_add_transition(first->end, EPSILON, epsilon);
	_add_transition(second->end, EPSILON, epsilon);
	first->end = epsilon;

	// create start state
	epsilon = _create_state();
	_add_transition(epsilon, EPSILON, first->start);
	_add_transition(epsilon, EPSILON, second->start);
	first->start = epsilon;

	free(second);
}

void operation_or(stack * stack_)
{
	assert(stack_ != NULL);
	assert(_stack_size(stack_) >= 2);

	stack_item_value value;
	_construction_block * first;
	_construction_block * second;

	value = _stack_pop(stack_);
	second = value.pointer;

	value = _stack_pop(stack_);
	first = value.pointer;

	operation_or_internal(first, second);

	value.pointer = first;
	_stack_push(stack_, value, POINTER);
}

void operation_star(stack * stack_)
{
	assert(stack_ != NULL);
	assert(_stack_size(stack_) >= 1);

	stack_item_value value;
	_construction_block * first;
	_state * epsilon = _create_state();

	value = _stack_pop(stack_);
	first = value.pointer;

	_add_transition(first->end, EPSILON, first->start);
	_add_transition(first->end, EPSILON, epsilon);
	first->end = epsilon;

	epsilon = _create_state();
	_add_transition(epsilon, EPSILON, first->start);
	first->start = epsilon;
	_add_transition(first->start, EPSILON, first->end);

	value.pointer = first;
	_stack_push(stack_, value, POINTER);
}

void enumeration(stack * stack_)
{
	assert(stack_ != NULL);
	assert(_stack_size(stack_) >= 2);

	short symbol;

	stack_item_value value;
	_construction_block * basic_block = _stack_pop(stack_).pointer;
	_construction_block * second_block;

	while((symbol = _stack_pop(stack_).number) != OPEN_BRACKET)
	{
		second_block = _create_block(symbol);
		operation_or_internal(basic_block, second_block);
	}

	value.pointer = basic_block;
	_stack_push(stack_, value, POINTER);
}

/**
 * @brief Return symbols, handle escapes
 */
short get_symbol(char * input, unsigned length, unsigned * position)
{
	short symbol;

	switch(input[*position])
	{
		case '\\': symbol = escape(input, length, position);break;
		case '[': symbol = OPEN_BRACKET;break;
		case ']': symbol = CLOSE_BRACKET;break;
		case '(': symbol = OPEN_PAREN;break;
		case ')': symbol = CLOSE_PAREN;break;
		case '+': symbol = PLUS; break;
		case '?': symbol = QUESTION_MARK;break;
		case '*': symbol = STAR;break;
		case '|': symbol = OR;break;
		case '.': symbol = DOT;break;
		default: symbol = input[*position];break;
	}

	++*position;
	return symbol;
}

/**
 * @brief create block for enumreation such as [ABCD]
 */
_construction_block * construct_enumeration(char * input, unsigned length, unsigned * position)
{
	short symbol = get_symbol(input, length, position);

	if(symbol > 128); //TODO goto hell;

	_construction_block * basic_block = _create_block(symbol);
	_construction_block * symbol_block;

	while(*position < length)
	{
		symbol = get_symbol(input, length, position);

		if(symbol == CLOSE_BRACKET) break;
		else if(symbol < 128) // regular characters
		{
			symbol_block = _create_block(symbol);
			operation_or_internal(basic_block, symbol_block);
		}
	}

	return basic_block;
}


/**
 * @brief Parse regular expression and generate DFA
 * @return [description]
 */
regex_pattern * parse(char * input, unsigned length)
{
	if(length == 0 || input == NULL) return 0;

	short symbol;
	short state;
	unsigned position;
	stack_item_value value;
	stack * stack_ = _stack_init();

	while(position < length)
	{
		symbol = get_symbol(input, length, &position);

		if(symbol == OPEN_BRACKET) // handle enumeration -> [ABCD]
		{
			value.pointer = construct_enumeration(input, length, &position);
			_stack_push(stack_, value, POINTER);
		}
	}

	return NULL;
}
