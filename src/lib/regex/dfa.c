#include "dfa.h"

char _escape(char * input, unsigned length, unsigned * position)
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
short _get_symbol(char * input, unsigned length, unsigned * position)
{
	short symbol;

	if(*position == length) return END;

	switch(input[*position])
	{
		case '\\': symbol = _escape(input, length, position);break;
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

_Bool _is_char(short symbol)
{
	return symbol < 128 || symbol == DOT;
}
short _validate(short current, short prev)
{
	if(_is_char(current))
	{
		current = SYMBOL;
	}

	if(_is_char(prev))
	{
		prev = SYMBOL;
	}

	return validation_table[current - 300][prev - 300];
}

void _nfa_state_free(_nfa_state * state)
{
	free(state->key);
	free(state->next);
	free(state->next_epsilon);
	free(state);
}

void _nfa_state_destroy(list * list_freed, _nfa_state * state)
{
	list_item_value value;
	value.pointer = state;

	list_append_front(list_freed, value, POINTER);

	for(unsigned i = 0; i < state->length; ++i)
	{
		value.pointer = state->next[i];
		if(list_search(list_freed, value, POINTER)) continue;
		_nfa_state_destroy(list_freed, value.pointer);
	}

	for(unsigned i = 0; i < state->length_epsilon; ++i)
	{
		value.pointer = state->next_epsilon[i];
		if(list_search(list_freed, value, POINTER)) continue;
		_nfa_state_destroy(list_freed, value.pointer);
	}

	_nfa_state_free(state);
}

void _nfa_block_destroy(void * block)
{
	list * list_freed = list_init(); //FIXME NULL possible

	_nfa_state_destroy(list_freed, ((_nfa_block *) block)->start);
	list_destroy(list_freed);
	free(block);
}


_nfa_state * _nfa_create_state()
{
	_nfa_state * state = malloc(sizeof(_nfa_state));

	if(state == NULL) return NULL;

	state->key = NULL;
	state->next = NULL;
	state->next_epsilon = NULL;
	state->length = 0;
	state->length_epsilon = 0;
	state->id = ID_NONE;

	return state;
}

_Bool _nfa_add_transition(_nfa_state * state, short symbol, _nfa_state * next)
{
	++state->length;
	state->key = realloc(state->key, state->length * sizeof(char)); //FIXME NULL possible
	state->next = realloc(state->next, state->length * sizeof(_nfa_state *)); //TODO data nechci ztratit, přiřazovat jinam
	state->key[state->length - 1] = symbol;
	state->next[state->length - 1] = next;

	return 1;
}

_Bool _dfa_add_transition(_dfa_state * state, short symbol, _dfa_state * next)
{
	++state->length;
	state->key = realloc(state->key, state->length * sizeof(char)); //FIXME NULL possible
	state->next = realloc(state->next, state->length * sizeof(_dfa_state *)); //TODO data nechci ztratit, přiřazovat jinam
	state->key[state->length - 1] = symbol;
	state->next[state->length - 1] = next;

	return 1;
}

_Bool _nfa_add_epsilon_transition(_nfa_state * state, _nfa_state * next)
{
	++state->length_epsilon; //FIXME NULL possible
	state->next_epsilon = realloc(state->next_epsilon, state->length_epsilon * sizeof(_nfa_state *)); //TODO data nechci ztratit, přiřazovat jinam
	state->next_epsilon[state->length_epsilon - 1] = next;

	return 1;
}

_nfa_block * _nfa_create_block(short symbol)
{
	_nfa_block * block = malloc(sizeof(_nfa_block));
	_Bool res;

	if(block == NULL) return NULL;

	block->start = _nfa_create_state();

	if(block->start == NULL)
	{
		free(block);
		return NULL;
	}

	block->end = _nfa_create_state();

	if(block->end == NULL)
	{
		free(block->start);
		free(block);
		return NULL;
	}

	res = _nfa_add_transition(block->start, symbol, block->end); //FIXME NULL possible

	if(res == 0)
	{
		_nfa_state_free(block->start);
		_nfa_state_free(block->end);
		free(block);

		return NULL;
	}

	return block;
}

_nfa_block * _nfa_create_dot_block()
{
	_nfa_block * block = _nfa_create_block(0); //FIXME NULL possible

	for(unsigned char i = 1; i < 128; ++i)
	{
		_nfa_add_transition(block->start, i, block->end); //FIXME NULL possible
		// TODO kontroloval res a když chyba tak break a odmazat všechny stavy
	}

	return block;
}

/**
 * @brief Concatenate two construction blocks
 * @param stack_ for construction blocks
 */
void _concat(stack * stack_)
{
	assert(stack_ != NULL);
	assert(_stack_size(stack_) >= 3);

	stack_item_value value;
	_nfa_block * first;
	_nfa_block * second;

	second = _stack_pop(stack_).pointer;
	value = _stack_pop(stack_);
	assert(value.number == CONCAT);
	first = _stack_pop(stack_).pointer;

	memcpy(first->end, second->start, sizeof(_nfa_state));
	first->end = second->end;

	free(second->start);
	free(second);

	value.pointer = first;
	_stack_push(stack_, value, POINTER);
}

void _or(stack * stack_)
{
	assert(stack_ != NULL);
	assert(_stack_size(stack_) >= 3);

	stack_item_value value;
	_nfa_block * first;
	_nfa_block * second;

	second = _stack_pop(stack_).pointer;
	value = _stack_pop(stack_);
	assert(value.number == OR);
	first = _stack_pop(stack_).pointer;

	for(unsigned i = 0; i < second->start->length; ++i)
	{
		_nfa_add_transition(first->start, second->start->key[i], second->start->next[i]); //FIXME NULL possible
	}

	for(unsigned i = 0; i < second->start->length_epsilon; ++i)
	{
		_nfa_add_epsilon_transition(first->start, second->start->next_epsilon[i]); //FIXME NULL possible
	}

	_nfa_add_epsilon_transition(second->end, first->end); //FIXME NULL possible

	_nfa_state_free(second->start);
	free(second);

	value.pointer = first;
	_stack_push(stack_, value, POINTER);
}

void _quantificator(stack * stack_, short operation)
{
	assert(stack_ != NULL);
	assert(_stack_size(stack_) >= 2);

	stack_item_value value;
	_nfa_block * block;
	_nfa_state * epsilon;

	value = _stack_pop(stack_);
	assert(value.number == operation);
	block = _stack_pop(stack_).pointer;


	if(operation != QUESTION_MARK)
	{
		epsilon = _nfa_create_state(); //FIXME NULL possible
		_nfa_add_epsilon_transition(block->end, block->start); //FIXME NULL possible
		_nfa_add_epsilon_transition(block->end, epsilon); //FIXME NULL possible
		block->end = epsilon;

		epsilon = _nfa_create_state(); //FIXME NULL possible
		_nfa_add_epsilon_transition(epsilon, block->start); //FIXME NULL possible
		block->start = epsilon;
	}

	if(operation != PLUS)
	{
		_nfa_add_epsilon_transition(block->start, block->end); //FIXME NULL possible
	}

	value.pointer = block;
	_stack_push(stack_, value, POINTER);
}

/**
 * @brief create block for enumreation such as [ABCD]
 */
_nfa_block * _enumeration(char * input, unsigned length, unsigned * position)
{
	short symbol = _get_symbol(input, length, position);

	if(!_is_char(symbol)) return NULL;

	_nfa_block * block = _nfa_create_block(symbol);

	if(block == NULL) return NULL;

	while(*position < length)
	{
		symbol = _get_symbol(input, length, position);

		if(symbol == CLOSE_BRACKET) return block;
		else if(!_is_char(symbol)) break;

		_nfa_add_transition(block->start, symbol, block->end); //FIXME NULL possible
	}

	_nfa_block_destroy(block);
	return NULL;
}


void * _nfa_clear(stack * stack_)
{
	_stack_free_pointers(stack_, _nfa_block_destroy);
	_stack_destroy(stack_);

	return NULL;
}

unsigned _precedence(stack * stack_, short current)
{
	if(_is_char(current) || current == DOT)
	{
		current = SYMBOL;
	}

	stack_item_value top = _stack_top_operation(stack_);

	return precedence_table[current - 300][top.number - 300];
}

void _reduce(stack * stack_)
{
	stack_item_value value;
	short symbol = _stack_top_operation(stack_).number;

	switch(symbol)
	{
		case OR:
			_or(stack_); //FIXME NULL possible
			break;

		case CONCAT:
			_concat(stack_); //FIXME NULL possible
			break;

		case STAR:
		case PLUS:
		case QUESTION_MARK:
			_quantificator(stack_, symbol); //FIXME NULL possible
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
_nfa_state * _nfa_construct(regex_pattern pattern)
{
	if(pattern.length == 0 || pattern.input == NULL) return 0;

	short actual_symbol, current = START, prev = START;
	_Bool loop_again = 0;
	unsigned position = 0;
	unsigned precedence_res;
	unsigned length = pattern.length;
	char * input = pattern.input;
	_nfa_state * result;
	_nfa_block * block;

	stack_item_value value;
	stack * stack_ = _stack_init(); //FIXME NULL possible

	value.number = STACK_BOTTOM;
	_stack_push(stack_, value, NUMBER);

	while(current != END)
	{
		actual_symbol = current = _get_symbol(input, length, &position);

		do
		{
			current = actual_symbol;
			loop_again = 0;

			switch(_validate(current, prev))
			{
				case FAIL:
					return _nfa_clear(stack_);
				case OK:
					break;
				case CONCAT:
					current = CONCAT;
					loop_again = 1;
					break;
			}

			while((precedence_res = _precedence(stack_, current)) == REDUCE)
			{
				_reduce(stack_); //FIXME NULL possible
			}

			if(precedence_res == FAIL) return _nfa_clear(stack_);
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
					value.pointer = _nfa_create_dot_block(); //FIXME NULL possible
					_stack_push(stack_, value, POINTER);
					break;

				case OPEN_BRACKET:
					value.pointer = _enumeration(input, length, &position); //FIXME NULL possible
					if(value.pointer == NULL) return _nfa_clear(stack_);
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
					value.pointer = _nfa_create_block(current); //FIXME NULL possible
					_stack_push(stack_, value, POINTER);
					prev = SYMBOL;
					break;
			}
		} while(loop_again);
	}

	while(_stack_top_operation(stack_).number != STACK_BOTTOM)
	{
		_reduce(stack_); //FIXME NULL possible
	}


	block = _stack_pop(stack_).pointer;
	block->end->id = pattern.id;
	result = block->start;

	if(_stack_top(stack_).number != STACK_BOTTOM) return _nfa_clear(stack_);

	_stack_destroy(stack_);
	free(block);

	return result;
}

_dfa_state * _dfa_create_state()
{
	_dfa_state * state = malloc(sizeof(_dfa_state)); //FIXME null

	state->length = 0;
	state->next = NULL;

	return state;

}

void _dfa_epsilon_closure(list * nfa)
{
	list_item * item = nfa->head;
	list_item_value value;
	unsigned length;

	while(item != NULL)
	{
		length = ((_nfa_state *) item->value.pointer)->length_epsilon;

		for(unsigned i = 0; i < length; ++i)
		{
			value.pointer = ((_nfa_state *) item->value.pointer)->next_epsilon[i];
			list_append_back(nfa, value, POINTER);
		}

		item = item->next;
	}
}

void _dfa_move(list * input_states, list * output_states, char character)
{
	list_clear(output_states);

	list_item * item = input_states->head;
	list_item_value value;
	_dfa_state * state;
	void * position;

	while(item != NULL)
	{
		state = item->value.pointer;

		if((position = memchr(state->key, character, state->length)) != NULL)
		{
			value.pointer = state->next[(char *) position - state->key];
			list_append_front(output_states, value, POINTER);
		}

		item = item->next;
	}
}

_dfa_pair * _dfa_create_pair(list * nfa_states)
{
	_dfa_pair * pair = malloc(sizeof(_dfa_pair)); // FIXME null
	pair->dfa_state = _dfa_create_state(); //FIXME null
	pair->nfa_states = nfa_states;

	return pair;
}

int _dfa_match_processed(list_item_value first, list_item_value second)
{
	list_item * first_item = ((_dfa_pair *)first.pointer)->nfa_states->head;
	list_item * second_item = ((_dfa_pair *)second.pointer)->nfa_states->head;

	unsigned first_size = ((_dfa_pair *)first.pointer)->nfa_states->size;
	unsigned second_size = ((_dfa_pair *)second.pointer)->nfa_states->size;

	if(first_size != second_size) return -1;

	while(first_item != NULL)
	{
		if(first_item->value.pointer == second_item->value.pointer) return 0;

		first_item = first_item->next;
	}

	return -1;
}

void * _dfa_convert(_nfa_state * root)
{
	list_item_value value;
	_dfa_pair * pair;
	_dfa_pair * new_pair;
	_dfa_pair * existing_pair;
	_dfa_state * dfa_root;
	list * nfa_list = list_init();
	list * move_states = list_init();
	list * pairs_processed = list_init();
	stack * pairs_unprocessed = _stack_init();

	value.pointer = root;
	list_append_front(nfa_list, value, POINTER);
	_dfa_epsilon_closure(nfa_list);

	pair = _dfa_create_pair(nfa_list);
	dfa_root = pair->dfa_state;

	value.pointer = pair;
	_stack_push(pairs_unprocessed, value, POINTER);

	while(!_stack_empty(pairs_unprocessed))
	{
		pair = _stack_pop(pairs_unprocessed).pointer;

		for(unsigned char i = 0; i <= 127; ++i)
		{
			_dfa_move(pair->nfa_states, move_states, i);
			_dfa_epsilon_closure(move_states);

			value.pointer = pair;
			existing_pair = list_find(pairs_processed, value, _dfa_match_processed)->pointer;

			if(existing_pair == NULL)
			{
				new_pair = _dfa_create_pair(move_states);
				value.pointer = new_pair;
				_stack_push(pairs_unprocessed, value, POINTER);

				_dfa_add_transition(pair->dfa_state, i, new_pair->dfa_state);
			}
			else
			{
				_dfa_add_transition(pair->dfa_state, i, existing_pair->dfa_state);
			}
		}


		value.pointer = pair;
		list_append_back(pairs_processed, value, POINTER);
	}

	return dfa_root;

}

regex_root * regex_construct(regex_pattern * patterns, unsigned count)
{
	unsigned i = 0;
	_nfa_state * regex;
	regex_root * root;



	for(i = 0; i < count; ++i)
	{
		regex = _nfa_construct(patterns[i]);

		if(regex == NULL)
		{
			root = NULL;
			// TODO vyřešit chyby
			break;
		}

		if(i == 0)
		{
			root = regex;
		}
		else
		{
			_nfa_add_epsilon_transition(root, regex);
		}
	}

	return root;
}

void regex_destroy(regex_root * root)
{
	if(root == NULL) return;

	list * list_freed = list_init(); //FIXME NULL possible

	_nfa_state_destroy(list_freed, root);
	list_destroy(list_freed);
}

int regex_match(regex_root * root, char * input, unsigned length)
{
	queue * start = _queue_init(); //FIXME NULL possible
	queue * end = _queue_init(); //FIXME NULL possible
	queue * swap_pointer;
	queue_item_value value;
	_nfa_state * state;
	unsigned position = 0;
	int result = -1;
	void * char_position;

	value.pointer = root;
	_queue_insert(start, value, POINTER); //FIXME NULL possible


	while(position < length)
	{
		while(!_queue_empty(start))
		{
			state = _queue_pop_front(start).pointer;

			for(unsigned i = 0; i < state->length_epsilon; ++i)
			{
				value.pointer = state->next_epsilon[i];
				_queue_insert(start, value, POINTER); //FIXME NULL possible
			}

			if((char_position = memchr(state->key, input[position], state->length))) // OR final state
			{
				value.pointer = state->next[char_position - (void *) state->key];
				_queue_insert(end, value, POINTER); //FIXME NULL possible
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
			_queue_insert(start, value, POINTER); //FIXME NULL possible
		}
	}

	_queue_destroy(start);
	_queue_destroy(end);

	return result;

}
