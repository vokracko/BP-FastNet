#include "dfa.h"

char _dfa_escape(char * input, unsigned length, unsigned * position)
{
	char character;

	if(length == *position + 1) return '\\';

	switch(input[*position + 1])
	{
		case '[': ++*position;character =  '[';break;
		case ']': ++*position;character =  ']';break;
		case '(': ++*position;character =  '(';break;
		case ')': ++*position;character =  ')';break;
		case '+': ++*position;character =  '+';break;
		case '?': ++*position;character =  '?';break;
		case '*': ++*position;character =  '*';break;
		case '|': ++*position;character =  '|';break;
		case '.': ++*position;character =  '.';break;
		default: character =  '\\';
	}

	return character;
}

/**
 * @brief Return symbols, handle escapes
 */
short _dfa_get_symbol(char * input, unsigned length, unsigned * position)
{
	short symbol;

	if(*position == length) return END;

	switch(input[*position])
	{
		case '\\': symbol = _dfa_escape(input, length, position);break;
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

_Bool _dfa_is_char(short symbol)
{
	return symbol < 128 || symbol == DOT;
}
short _dfa_validate(short current, short prev)
{
	if(_dfa_is_char(current))
	{
		current = SYMBOL;
	}

	if(_dfa_is_char(prev))
	{
		prev = SYMBOL;
	}

	return validation_table[current - 300][prev - 300];
}

void _dfa_state_free(_dfa_state * state)
{
	free(state->key);
	free(state->next);
	free(state->next_epsilon);
	free(state);
}

void _dfa_state_destroy(list * list_freed, _dfa_state * state)
{
	list_item_value value;
	value.pointer = state;

	list_append_front(list_freed, value, POINTER);

	for(unsigned i = 0; i < state->length; ++i)
	{
		value.pointer = state->next[i];
		if(list_search(list_freed, value, POINTER)) continue;
		_dfa_state_destroy(list_freed, value.pointer);
	}

	for(unsigned i = 0; i < state->length_epsilon; ++i)
	{
		value.pointer = state->next_epsilon[i];
		if(list_search(list_freed, value, POINTER)) continue;
		_dfa_state_destroy(list_freed, value.pointer);
	}

	_dfa_state_free(state);
}

void _dfa_block_destroy(void * block)
{
	list * list_freed = list_init();

	_dfa_state_destroy(list_freed, ((_dfa_block *) block)->start);
	list_destroy(list_freed);
	free(block);
}


_dfa_state * _dfa_create_state()
{
	_dfa_state * state = malloc(sizeof(_dfa_state));

	if(state == NULL) return NULL;

	state->key = NULL;
	state->next = NULL;
	state->next_epsilon = NULL;
	state->length = 0;
	state->length_epsilon = 0;
	state->id = ID_NONE;

	return state;
}

_Bool _dfa_add_transition(_dfa_state * state, short symbol, _dfa_state * next)
{
	++state->length;
	state->key = realloc(state->key, state->length * sizeof(char));
	state->next = realloc(state->next, state->length * sizeof(_dfa_state *)); //TODO data nechci ztratit, přiřazovat jinam
	state->key[state->length - 1] = symbol;
	state->next[state->length - 1] = next;

	return 1;
}

_Bool _dfa_add_epsilon_transition(_dfa_state * state, _dfa_state * next)
{
	++state->length_epsilon;
	state->next_epsilon = realloc(state->next_epsilon, state->length_epsilon * sizeof(_dfa_state *)); //TODO data nechci ztratit, přiřazovat jinam
	state->next_epsilon[state->length_epsilon - 1] = next;

	return 1;
}

_dfa_block * _dfa_create_block(short symbol)
{
	_dfa_block * block = malloc(sizeof(_dfa_block));
	_Bool res;

	if(block == NULL) return NULL;

	block->start = _dfa_create_state();

	if(block->start == NULL)
	{
		free(block);
		return NULL;
	}

	block->end = _dfa_create_state();

	if(block->end == NULL)
	{
		free(block->start);
		free(block);
		return NULL;
	}

	res = _dfa_add_transition(block->start, symbol, block->end);

	if(res == 0)
	{
		_dfa_state_free(block->start);
		_dfa_state_free(block->end);
		free(block);

		return NULL;
	}

	return block;
}

_dfa_block * _dfa_create_dot_block()
{
	_dfa_block * block = _dfa_create_block(0);

	for(unsigned char i = 1; i < 128; ++i)
	{
		_dfa_add_transition(block->start, i, block->end);
		// TODO kontroloval res a když chyba tak break a odmazat všechny stavy
	}

	return block;
}

/**
 * @brief Concatenate two construction blocks
 * @param stack_ for construction blocks
 */
void _dfa_concat(stack * stack_)
{
	assert(stack_ != NULL);
	assert(_stack_size(stack_) >= 3);

	stack_item_value value;
	_dfa_block * first;
	_dfa_block * second;

	second = _stack_pop(stack_).pointer;
	value = _stack_pop(stack_);
	assert(value.number == CONCAT);
	first = _stack_pop(stack_).pointer;

	memcpy(first->end, second->start, sizeof(_dfa_state));
	first->end = second->end;

	free(second->start);
	free(second);

	value.pointer = first;
	_stack_push(stack_, value, POINTER);
}

void _dfa_or(stack * stack_)
{
	assert(stack_ != NULL);
	assert(_stack_size(stack_) >= 3);

	stack_item_value value;
	_dfa_block * first;
	_dfa_block * second;

	second = _stack_pop(stack_).pointer;
	value = _stack_pop(stack_);
	assert(value.number == OR);
	first = _stack_pop(stack_).pointer;

	for(unsigned i = 0; i < second->start->length; ++i)
	{
		_dfa_add_transition(first->start, second->start->key[i], second->start->next[i]);
	}

	for(unsigned i = 0; i < second->start->length_epsilon; ++i)
	{
		_dfa_add_epsilon_transition(first->start, second->start->next_epsilon[i]);
	}

	_dfa_add_epsilon_transition(second->end, first->end);

	_dfa_state_free(second->start);
	free(second);

	value.pointer = first;
	_stack_push(stack_, value, POINTER);
}

void _dfa_quantificator(stack * stack_, short operation)
{
	assert(stack_ != NULL);
	assert(_stack_size(stack_) >= 2);

	stack_item_value value;
	_dfa_block * block;
	_dfa_state * epsilon;

	value = _stack_pop(stack_);
	assert(value.number == operation);
	block = _stack_pop(stack_).pointer;


	if(operation != QUESTION_MARK)
	{
		epsilon = _dfa_create_state();
		_dfa_add_epsilon_transition(block->end, block->start);
		_dfa_add_epsilon_transition(block->end, epsilon);
		block->end = epsilon;

		epsilon = _dfa_create_state();
		_dfa_add_epsilon_transition(epsilon, block->start);
		block->start = epsilon;
	}

	if(operation != PLUS)
	{
		_dfa_add_epsilon_transition(block->start, block->end);
	}

	value.pointer = block;
	_stack_push(stack_, value, POINTER);
}

/**
 * @brief create block for enumreation such as [ABCD]
 */
_dfa_block * _dfa_enumeration(char * input, unsigned length, unsigned * position)
{
	short symbol = _dfa_get_symbol(input, length, position);

	if(!_dfa_is_char(symbol)) return NULL;

	_dfa_block * block = _dfa_create_block(symbol);

	if(block == NULL) return NULL;

	while(*position < length)
	{
		symbol = _dfa_get_symbol(input, length, position);

		if(symbol == CLOSE_BRACKET) return block;
		else if(!_dfa_is_char(symbol)) break;

		_dfa_add_transition(block->start, symbol, block->end);
	}

	_dfa_block_destroy(block);
	return NULL;
}


void * _dfa_clear(stack * stack_)
{
	_stack_free_pointers(stack_, _dfa_block_destroy);
	_stack_destroy(stack_);

	return NULL;
}

unsigned _dfa_precedence(stack * stack_, short current)
{
	if(_dfa_is_char(current) || current == DOT)
	{
		current = SYMBOL;
	}

	stack_item_value top = _stack_top_operation(stack_);

	return precedence_table[current - 300][top.number - 300];
}

void _dfa_reduce(stack * stack_)
{
	stack_item_value value;
	short symbol = _stack_top_operation(stack_).number;

	switch(symbol)
	{
		case OR:
			_dfa_or(stack_);
			break;

		case CONCAT:
			_dfa_concat(stack_);
			break;

		case STAR:
		case PLUS:
		case QUESTION_MARK:
			_dfa_quantificator(stack_, symbol);
			break;

		case OPEN_PAREN:
			value = _stack_pop(stack_); // pop item
			_stack_pop(stack_); // pop OPEN_PAREN
			_stack_push(stack_, value, POINTER);
			break;
	}
}

/**
 * @brief Parse regular expression and generate DFA
 * @return [description]
 */
_dfa_state * _dfa_construct(regex_pattern pattern)
{
	if(pattern.length == 0 || pattern.input == NULL) return 0;

	short actual_symbol, current = START, prev = START;
	_Bool loop_again = 0;
	unsigned position = 0;
	unsigned precedence_res;
	unsigned length = pattern.length;
	char * input = pattern.input;
	_dfa_state * result;
	_dfa_block * block;

	stack_item_value value;
	stack * stack_ = _stack_init();

	value.number = STACK_BOTTOM;
	_stack_push(stack_, value, NUMBER);

	while(current != END)
	{
		actual_symbol = current = _dfa_get_symbol(input, length, &position);

		do
		{
			current = actual_symbol;
			loop_again = 0;

			switch(_dfa_validate(current, prev))
			{
				case FAIL:
					return _dfa_clear(stack_);
				case OK:
					break;
				case CONCAT:
					current = CONCAT;
					loop_again = 1;
					break;
			}

			// FIXME current 310, prev = 313 vrací fail místo reduce
			while((precedence_res = _dfa_precedence(stack_, current)) == REDUCE)
			{
				_dfa_reduce(stack_);
			}

			if(precedence_res == FAIL) return _dfa_clear(stack_);
			else if(precedence_res == FIN) break;
			else if(precedence_res == EQUAL) // only in case current = CLOSE_PAREN && stack_top == OPEN_PAREN
			{
				value = _stack_pop(stack_); // pop block
				_stack_pop(stack_); // pop OPEN_PAREN
				_stack_push(stack_, value, POINTER);
				prev = CLOSE_PAREN;
			}

			switch(current)
			{


				case CLOSE_PAREN:
					break;

				case DOT:
					prev = DOT;
					value.pointer = _dfa_create_dot_block();
					_stack_push(stack_, value, POINTER);
					break;

				case OPEN_BRACKET:
					value.pointer = _dfa_enumeration(input, length, &position);
					if(value.pointer == NULL) return _dfa_clear(stack_);
					_stack_push(stack_, value, POINTER);
					prev = CLOSE_BRACKET;
					break;

				case CONCAT:
				case OPEN_PAREN:
				case STAR:
				case QUESTION_MARK:
				case OR:
				case PLUS:
					prev = current;
					value.number = current;
					_stack_push(stack_, value, NUMBER);
					break;

				default:
					// printf("%c\n", current);
					value.pointer = _dfa_create_block(current);
					_stack_push(stack_, value, POINTER);
					prev = SYMBOL;
					break;
			}
		} while(loop_again);
	}

	while(_stack_top_operation(stack_).number != STACK_BOTTOM)
	{
		_dfa_reduce(stack_);
	}


	block = _stack_pop(stack_).pointer;
	block->end->id = pattern.id;
	result = block->start;

	if(_stack_top(stack_).number != STACK_BOTTOM) return _dfa_clear(stack_);

	_stack_destroy(stack_);
	free(block);

	return result;
}

regex_root * regex_construct(regex_pattern * patterns, unsigned count)
{
	unsigned i = 0;
	regex_root * root = malloc(sizeof(regex_root));

	if(root == NULL) return NULL;

	root->patterns = malloc(sizeof(_dfa_state) * count);

	if(root->patterns == NULL) return NULL;

	_dfa_state * regex;



	for(i = 0; i < count; ++i)
	{
		regex = _dfa_construct(patterns[i]);

		if(regex == NULL)
		{
			// TODO nastavit errno
			break;
		}

		root->patterns[i] = regex;
	}

	root->count = i;
	return root;
}

void regex_destroy(regex_root * root)
{
	if(root == NULL) return;

	list * list_freed = list_init();

	for(unsigned i = 0; i < root->count; ++i)
	{
		_dfa_state_destroy(list_freed, root->patterns[i]);
	}

	list_destroy(list_freed);

	free(root->patterns);
	free(root);
}

int regex_match(regex_root * root, char * input, unsigned length)
{
	queue * start = _queue_init();
	queue * end = _queue_init();
	queue * swap_pointer;
	queue_item_value value;
	_dfa_state * state;
	unsigned position = 0;
	int result = -1;
	void * char_position;

	for(unsigned i = 0; i < root->count; ++i)
	{
		value.pointer = root->patterns[i];
		_queue_insert(start, value, POINTER);
	}

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
				value.pointer = state->next[char_position - (void *) state->key];
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

		if(state->id != ID_NONE)
		{
			result = state->id;
			break;
		}

		for(unsigned i = 0; i < state->length_epsilon; ++i)
		{
			value.pointer = state->next_epsilon[i];
			_queue_insert(start, value, POINTER);
		}
	}

	_queue_destroy(start);
	_queue_destroy(end);

	return result;

}
