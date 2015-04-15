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

/**
 * @brief Return symbols, handle escapes
 */
short get_symbol(char * input, unsigned length, unsigned * position)
{
	short symbol;

	if(*position == length) return END;

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

_Bool is_char(short symbol)
{
	return symbol < 128;
}

_Bool is_quantificator(short symbol)
{
	return
		symbol == QUESTION_MARK ||
		symbol == PLUS ||
		symbol == STAR;
}

_Bool is_valid(short current, short prev)
{
	if(is_quantificator(current) && (is_quantificator(prev) || prev == '|' || prev == '(')) return 0;
	else if(current == ']' && !is_char(prev)) return 0;
	else if(current == ')' && (prev == ')' || prev == '|')) return 0;
	else if(current == '|' && (prev == '(' || prev == '|')) return 0;
	else if((is_quantificator(current) || current == ']' || current == ')' || current == '|') && prev == START) return 0;

	return 1;
}

_Bool already_freed(list * list_freed, void * pointer)
{
	list_item * item = list_freed->head;

	while(item != NULL && item->value.pointer != pointer)
	{
		item = item->next;
	}

	return item != NULL;
}

void state_free(_state * state)
{
	free(state->key);
	free(state->next);
	free(state->next_epsilon);
	free(state);
}

void state_destroy(list * list_freed, _state * state)
{
	list_item_value value;
	value.pointer = state;

	list_append_front(list_freed, value, POINTER);

	for(unsigned i = 0; i < state->length; ++i)
	{
		if(already_freed(list_freed, state->next[i])) continue;
		state_destroy(list_freed, state->next[i]);
	}

	for(unsigned i = 0; i < state->length_epsilon; ++i)
	{
		if(already_freed(list_freed, state->next_epsilon[i])) continue;
		state_destroy(list_freed, state->next_epsilon[i]);
	}

	state_free(state);
}

void block_destroy(_construction_block * block)
{
	list * list_freed = list_init();

	state_destroy(list_freed, block->start);
	list_destroy(list_freed);
	free(block);
}


_state * _create_state()
{
	_state * state = malloc(sizeof(_state));

	if(state == NULL) return NULL;

	state->key = NULL;
	state->next = NULL;
	state->next_epsilon = NULL;
	state->length = 0;
	state->length_epsilon = 0;
	state->end_state = NO_END;

	return state;
}

_Bool _add_transition(_state * state, short symbol, _state * next)
{
	++state->length;
	state->key = realloc(state->key, state->length * sizeof(char));
	state->next = realloc(state->next, state->length * sizeof(_state *)); //TODO data nechci ztratit, přiřazovat jinam
	state->key[state->length - 1] = symbol;
	state->next[state->length - 1] = next;

	return 1;
}

_Bool _add_epsilon_transition(_state * state, _state * next)
{
	++state->length_epsilon;
	state->next_epsilon = realloc(state->next_epsilon, state->length_epsilon * sizeof(_state *)); //TODO data nechci ztratit, přiřazovat jinam
	state->next_epsilon[state->length_epsilon - 1] = next;

	return 1;
}

_construction_block * _create_block(short symbol)
{
	_construction_block * block = malloc(sizeof(_construction_block));
	_Bool res;

	if(block == NULL) return NULL;

	block->start = _create_state();

	if(block->start == NULL)
	{
		free(block);
		return NULL;
	}

	block->end = _create_state();

	if(block->end == NULL)
	{
		free(block->start);
		free(block);
		return NULL;
	}

	res = _add_transition(block->start, symbol, block->end);

	if(res == 0)
	{
		state_free(block->start);
		state_free(block->end);
		free(block);

		return NULL;
	}

	return block;
}

_construction_block * _create_dot_block()
{
	_construction_block * basic_block = _create_block(0);
	_construction_block * symbol_block;

	for(unsigned char i = 1; i < 128; ++i)
	{
		symbol_block = _create_block(i);
		operation_or_internal(basic_block, symbol_block);
		// TODO kontroloval res a když chyba tak break a odmazat všechny stavy
	}

	return basic_block;
}

/**
 * @brief Concatenate two construction blocks
 * @param stack_ for construction blocks
 */
void operation_concatenation(stack * stack_state)
{
	assert(stack_state != NULL);
	assert(_stack_size(stack_state) >= 2);

	stack_item_value value;
	_construction_block * first;
	_construction_block * second;

	second = _stack_pop(stack_state).pointer;
	first = _stack_pop(stack_state).pointer;

	_add_epsilon_transition(first->end, second->start);
	first->end = second->start;

	free(second);

	value.pointer = first;
	_stack_push(stack_state, value, POINTER);
}

void operation_or_internal(_construction_block * first, _construction_block * second)
{
	_state * epsilon = _create_state();
	// create end state
	_add_epsilon_transition(first->end, epsilon);
	_add_epsilon_transition(second->end, epsilon);
	first->end = epsilon;

	// create start state
	epsilon = _create_state();
	_add_epsilon_transition(epsilon, first->start);
	_add_epsilon_transition(epsilon, second->start);
	first->start = epsilon;

	free(second);
}

void operation_or(stack * stack_state)
{
	assert(stack_state != NULL);
	assert(_stack_size(stack_state) >= 2);

	stack_item_value value;
	_construction_block * first;
	_construction_block * second;

	second = _stack_pop(stack_state).pointer;
	first = _stack_pop(stack_state).pointer;

	operation_or_internal(first, second);

	value.pointer = first;
	_stack_push(stack_state, value, POINTER);
}

void operation_star(stack * stack_state)
{
	assert(stack_state != NULL);
	assert(_stack_size(stack_state) >= 1);

	stack_item_value value;
	_construction_block * first;
	_state * epsilon = _create_state();

	first = _stack_pop(stack_state).pointer;

	_add_epsilon_transition(first->end, first->start);
	_add_epsilon_transition(first->end, epsilon);
	first->end = epsilon;

	epsilon = _create_state();
	_add_epsilon_transition(epsilon, first->start);
	first->start = epsilon;
	_add_epsilon_transition(first->start, first->end);

	value.pointer = first;
	_stack_push(stack_state, value, POINTER);
}

/**
 * @brief create block for enumreation such as [ABCD]
 */
_construction_block * construct_enumeration(char * input, unsigned length, unsigned * position)
{
	short symbol = get_symbol(input, length, position);

	if(!is_char(symbol)) return NULL;

	_construction_block * basic_block = _create_block(symbol);
	_construction_block * symbol_block;

	if(basic_block == NULL) return NULL;

	while(*position < length)
	{
		symbol = get_symbol(input, length, position);

		if(symbol == CLOSE_BRACKET) break;
		else if(!is_char(symbol))
		{
			block_destroy(basic_block);
			return NULL;
		}

		symbol_block = _create_block(symbol);
		operation_or_internal(basic_block, symbol_block);
	}

	return basic_block;
}


_Bool is_concat(short current, short prev)
{
	return
		(
			prev == QUESTION_MARK ||
			prev == PLUS ||
			prev == STAR ||
			is_char(prev) ||
			prev == CLOSE_BRACKET ||
			prev == CLOSE_PAREN ||
			prev == DOT
		)
		&&
		(
			is_char(current) ||
			current == OPEN_BRACKET ||
			current == OPEN_PAREN ||
			current == DOT
		);
}

void maybe_concat(stack * stack_operation, short current, short prev)
{
	stack_item_value value;

	if(is_concat(current, prev))
	{
		value.number = CONCATENATION;
		_stack_push(stack_operation, value, NUMBER);
	}
}

void push_char(stack * stack_state, short symbol)
{
	_construction_block * block;

	if(symbol == DOT)
	{
		block = _create_dot_block();
	}
	else
	{
		block = _create_block(symbol);
	}

	stack_item_value value;
	value.pointer = block;

	_stack_push(stack_state, value, POINTER);
}

void * clear(stack * stack_operation, stack * stack_state)
{
	_stack_destroy(stack_operation);

	while(!_stack_empty(stack_state))
	{
		block_destroy(_stack_pop(stack_state).pointer);
	}

	_stack_destroy(stack_state);

	return NULL;
}

unsigned precedence(stack * stack_operation, short symbol)
{
	stack_item_value item = _stack_top(stack_operation);

	return precedence_table[symbol - 300][item.number - 300];
}

void reduce(stack * stack_operation, stack * stack_state)
{
	switch(_stack_pop(stack_operation).number)
	{
		case OR:
			operation_or(stack_state);
			break;

		case CONCATENATION:
			operation_concatenation(stack_state);
			break;

		case STAR:
			operation_star(stack_state);
			break;

		case OPEN_PAREN:
			break;

	}

	// TODO možná chyba -- na vrcholu zásobníku je stack bottom
}

/**
 * @brief Parse regular expression and generate DFA
 * @return [description]
 */
regex_pattern * parse(char * input, unsigned length, unsigned char regex_number)
{
	if(length == 0 || input == NULL) return 0;

	regex_pattern * result;
	short symbol = START;
	short prev = START; // cokoli co nebude mít za následek generování konkatenace
	unsigned position = 0;
	unsigned precedence_res;
	stack_item_value value;
	stack * stack_state = _stack_init();
	stack * stack_operation = _stack_init();
	_construction_block * block;

	value.number = STACK_BOTTOM;
	_stack_push(stack_operation, value, NUMBER);

	while(symbol != END)
	{
		symbol = get_symbol(input, length, &position);

		if(!is_valid(symbol, prev)) return clear(stack_operation, stack_state);

		if(is_char(symbol) || symbol == DOT)
		{
			push_char(stack_state, symbol);
		}
		else
		{
			while((precedence_res = precedence(stack_operation, symbol)) == OP_REDUCE)
			{
				reduce(stack_operation, stack_state);
			}

			if(precedence_res == OP_NONE) return clear(stack_operation, stack_state);
			else if(precedence_res == OP_FIN) break;
			else if(precedence_res == OP_EQUAL)
			{
				_stack_pop(stack_operation);
				prev = symbol;
				continue;
			}

			if(symbol == OPEN_BRACKET) // handle enumeration -> [ABCD]
			{
				value.pointer = construct_enumeration(input, length, &position);

				if(value.pointer == NULL) return clear(stack_operation, stack_state);

				_stack_push(stack_state, value, POINTER);
				maybe_concat(stack_operation, OPEN_BRACKET, prev);
				prev = CLOSE_BRACKET;
				continue;
			}

			if(symbol == OPEN_PAREN)
			{
				maybe_concat(stack_operation, OPEN_PAREN, prev);
				value.number = symbol;
				_stack_push(stack_operation, value, NUMBER);
				prev = OPEN_PAREN;
				continue;
			}

			value.number = symbol;
			_stack_push(stack_operation, value, NUMBER);
		}

		maybe_concat(stack_operation, symbol, prev);
		prev = symbol;
	}

	while(_stack_top(stack_operation).number != STACK_BOTTOM)
	{
		reduce(stack_state, stack_state);
	}


	block = _stack_pop(stack_state).pointer;
	block->end->end_state = regex_number;
	result = block->start;

	if(_stack_size(stack_state) != 0) return clear(stack_operation, stack_state);

	_stack_destroy(stack_operation);
	_stack_destroy(stack_state);
	free(block);

	return result;
}

void regex_free(regex_pattern * pattern)
{
	list * list_freed = list_init();

	state_destroy(list_freed, pattern);
	list_destroy(list_freed);
}

int match(regex_pattern * pattern, char * input, unsigned length)
{
	queue * start = _queue_init();
	queue * end = _queue_init();
	queue * swap_pointer;
	queue_item_value value;
	_state * state;
	unsigned position = 0;
	int result = -1;
	void * char_position;

	value.pointer = pattern;
	_queue_insert(start, value, POINTER);

	while(position < length)
	{
		while(!_queue_empty(start))
		{
			state = _queue_pop_front(start).pointer;

			for(unsigned i = 0; i < state->length_epsilon; ++i)
			{
				value.pointer = state->next_epsilon[i];
				_queue_insert(start, value, POINTER);
			}

			if((char_position = memchr(state->key, input[position], state->length))) // OR final state
			{
				value.pointer = state->next[(void *) state->key - char_position];
				_queue_insert(end, value, POINTER);
			}
		}

		swap_pointer = end;
		end = start;
		start = swap_pointer;

 		++position;
	}

	while(!_queue_empty(start))
	{
		state = _queue_pop_front(start).pointer;

		for(unsigned i = 0; i < state->length_epsilon; ++i)
		{
			value.pointer = state->next_epsilon[i];
			_queue_insert(start, value, POINTER);
		}


		if(state->end_state != NO_END)
		{
			result = state->end_state;
			break;
		}
	}

	return result;

}
