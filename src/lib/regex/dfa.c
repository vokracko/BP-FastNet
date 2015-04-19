#include "dfa.h"

void * _out_of_memory()
{
	errno = REGEX_OUT_OF_MEMORY;
	return NULL;
}

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

_Bool _add_transition(char ** key, void *** next, unsigned * length, short symbol, void * next_pointer)
{
	char * new_key;
	void ** new_next;

	new_key = realloc(*key, (*length + 1) * sizeof(char));

	if(new_key == NULL) return 0;

	new_next = realloc(*next, (*length + 1) * sizeof(void *));

	if(new_next == NULL)
	{
		free(new_key);
		return 0;
	}

	new_key[*length] = symbol;
	new_next[*length] = next_pointer;

	*key = new_key;
	*next = new_next;

	++(*length);

	return 1;
}

_Bool _nfa_add_transition(_nfa_state * state, short symbol, _nfa_state * next)
{
	return _add_transition(&(state->key), (void ***) &(state->next), &(state->length), symbol, next);
}

_Bool _dfa_add_transition(_dfa_state * state, short symbol, _dfa_state * next)
{
	return _add_transition(&(state->key), (void ***) &(state->next), &(state->length), symbol, next);
}

_Bool _nfa_add_epsilon_transition(_nfa_state * state, _nfa_state * next)
{
	_nfa_state ** new_next_epsilon;

	new_next_epsilon = realloc(state->next_epsilon, (state->length_epsilon + 1) * sizeof(_nfa_state *));

	if(new_next_epsilon == NULL) return 0;

	new_next_epsilon[state->length_epsilon] = next;
	state->next_epsilon = new_next_epsilon;

	++state->length_epsilon;
	return 1;
}

_nfa_state * _nfa_create_state()
{
	_nfa_state * state = malloc(sizeof(_nfa_state));

	if(state == NULL) return _out_of_memory();

	state->key = NULL;
	state->next = NULL;
	state->next_epsilon = NULL;
	state->length = 0;
	state->length_epsilon = 0;
	state->id = ID_NONE;

	return state;
}

_dfa_state * _dfa_create_state()
{
	_dfa_state * state = malloc(sizeof(_dfa_state)); //FIXME null

	if(state == NULL) return _out_of_memory();

	state->length = 0;
	state->next = NULL;
	state->key = NULL;
	state->id = ID_NONE;

	return state;
}

void _nfa_state_free(_nfa_state * state)
{
	free(state->key);
	free(state->next);
	free(state->next_epsilon);
	free(state);
}

void _dfa_state_free(_dfa_state * state)
{
	assert(state != NULL);

	free(state->key);
	free(state->next);
	free(state);
}

void _nfa_state_destroy(list * list_freed, _nfa_state * state)
{
	list_item_value value;
	value.pointer = state;

	/*
		Program can enter this part if it runs out of memory
	 	first free sizeof(state) && length(state->key) which is bigger than
	 	sizeof(list_item) => program can proceed succesfully
	*/
	unsigned length = state->length;
	unsigned length_epsilon = state->length_epsilon;
	_nfa_state ** next = state->next;
	_nfa_state ** next_epsilon = state->next_epsilon;
	free(state->key);
	free(state);

	list_append_front(list_freed, value, POINTER);

	for(unsigned i = 0; i < length; ++i)
	{
		value.pointer = next[i];
		if(list_contains(list_freed, value, POINTER)) continue;
		_nfa_state_destroy(list_freed, value.pointer);
	}

	for(unsigned i = 0; i < length_epsilon; ++i)
	{
		value.pointer = next_epsilon[i];
		if(list_contains(list_freed, value, POINTER)) continue;
		_nfa_state_destroy(list_freed, value.pointer);
	}

	free(next);
	free(next_epsilon);
}

void _dfa_state_destroy(list * list_freed, _dfa_state * state)
{
	list_item_value value;
	value.pointer = state;

	/*
		Program can enter this part if it runs out of memory
	 	first free sizeof(state) && length(state->key) which is bigger than
	 	sizeof(list_item) => program can proceed succesfully
	*/
	unsigned length = state->length;
	_dfa_state ** next = state->next;
	free(state->key);
	free(state);

	list_append_front(list_freed, value, POINTER);

	for(unsigned i = 0; i < length; ++i)
	{
		value.pointer = next[i];
		if(list_contains(list_freed, value, POINTER)) continue;
		_dfa_state_destroy(list_freed, value.pointer);
	}

	free(next);
}

void _dfa_destroy(_dfa_state * state)
{
	list list_freed;

	bzero(&list_freed, sizeof(list));
	_dfa_state_destroy(&list_freed, state);
	list_clear(&list_freed);
}

void _nfa_destroy(_nfa_state * state)
{
	list list_freed;

	bzero(&list_freed, sizeof(list));
	_nfa_state_destroy(&list_freed, state);
	list_clear(&list_freed);
}


_nfa_block * _nfa_create_block(short symbol)
{
	_nfa_block * block = malloc(sizeof(_nfa_block));
	_Bool res;

	if(block == NULL) return _out_of_memory();

	block->start = _nfa_create_state();

	if(block->start == NULL)
	{
		free(block);
		return _out_of_memory();
	}

	block->end = _nfa_create_state();

	if(block->end == NULL)
	{
		free(block->start);
		free(block);
		return _out_of_memory();
	}

	res = _nfa_add_transition(block->start, symbol, block->end); //FIXME NULL possible

	if(res == 0)
	{
		_nfa_state_free(block->start);
		_nfa_state_free(block->end);
		free(block);

		return _out_of_memory();
	}

	return block;
}

_nfa_block * _nfa_create_dot_block()
{
	_nfa_block * block = _nfa_create_block(0); //FIXME NULL possible
	_Bool res;
	if(block == NULL) return _out_of_memory();

	for(int i = CHAR_MIN; i < CHAR_MAX; ++i)
	{
		res = _nfa_add_transition(block->start, i, block->end);

		if(res == 0)
		{
			_nfa_state_free(block->start);
			_nfa_state_free(block->end);
			free(block);
			return _out_of_memory();
		}
	}

	return block;
}

_dfa_pair * _dfa_create_pair(list * nfa_states)
{
	list_item * item = nfa_states->head;
	unsigned char id;
	_dfa_pair * pair = malloc(sizeof(_dfa_pair));

	if(pair == NULL) return _out_of_memory();

	pair->dfa_state = _dfa_create_state();

	if(pair->dfa_state == NULL)
	{
		free(pair);
		return _out_of_memory();
	}

	while(item != NULL)
	{
		id = ((_nfa_state *) item->value.pointer)->id;
		if(id != ID_NONE)
		{
			pair->dfa_state->id = id;
			break;
		}

		item = item->next;
	}

	pair->nfa_states = nfa_states;

	return pair;
}

void _nfa_block_destroy(void * block)
{
	_nfa_state * start_state = ((_nfa_block *) block)->start;

	free(block);

	_nfa_destroy(start_state);
}

void _dfa_pair_free(void * pair)
{
	list_destroy(((_dfa_pair *) pair)->nfa_states);
	free(pair);
}

void _dfa_pair_destroy(void * pair)
{
	_dfa_state_free(((_dfa_pair *) pair)->dfa_state);
	_dfa_pair_free(pair);
}

/**
 * @brief Construction of nfa failed, clear all items and return null to singalize error
 * @return NULL
 */
void * _nfa_clear(stack * stack_)
{
	_stack_free_pointers(stack_, _nfa_block_destroy);
	_stack_destroy(stack_);

	return NULL;
}

void * _dfa_clear(_nfa_state * root, list * nfa_list, list * move_states, list * pairs_created, list * pairs_unprocessed, _Bool free_dfa_states)
{
	regex_destroy_nfa(root);

	list_destroy(nfa_list);
	list_destroy(move_states);

	if(free_dfa_states) list_free_pointers(pairs_created, _dfa_pair_destroy);
	else list_free_pointers(pairs_created, _dfa_pair_free);
	list_destroy(pairs_created);
	list_free_pointers(pairs_unprocessed, _dfa_pair_destroy);
	list_destroy(pairs_unprocessed);

	return NULL;
}

/**
 * @brief Concatenate two construction blocks
 * @param stack_ for construction blocks
 */
_Bool _concat(stack * stack_)
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

	// _nfa_add_epsilon_transition(first->end, second->start);

	memcpy(first->end, second->start, sizeof(_nfa_state));
	first->end = second->end;

	free(second->start);
	free(second);

	value.pointer = first;
	if(_stack_push(stack_, value, POINTER) == 0)
	{
		_nfa_block_destroy(value.pointer);
		return 0;
	}

	return 1;
}

_Bool _or(stack * stack_)
{
	assert(stack_ != NULL);
	assert(_stack_size(stack_) >= 3);

	stack_item_value value;
	_nfa_block * first;
	_nfa_block * second;
	_Bool res;

	second = _stack_pop(stack_).pointer;
	value = _stack_pop(stack_);
	assert(value.number == OR);
	first = _stack_pop(stack_).pointer;

	res = _nfa_add_epsilon_transition(first->start, second->start);

	if(res == 0)
	{
		_nfa_block_destroy(first);
		_nfa_block_destroy(second);
		return 0;
	}

	res = _nfa_add_epsilon_transition(second->end, first->end);
	free(second);

	if(res == 0)
	{
		_nfa_block_destroy(first);
		return 0;
	}

	value.pointer = first;

	if(_stack_push(stack_, value, POINTER) == 0)
	{
		_nfa_block_destroy(first);
		return 0;
	}

	return 1;
}

_Bool _quantificator(stack * stack_, short operation)
{
	assert(stack_ != NULL);
	assert(_stack_size(stack_) >= 2);

	stack_item_value value;
	_nfa_block * block;
	_nfa_state * epsilon;
	_Bool res;

	value = _stack_pop(stack_);
	assert(value.number == operation);
	block = _stack_pop(stack_).pointer;

	if(operation != QUESTION_MARK)
	{
		res = _nfa_add_epsilon_transition(block->end, block->start);

		if(res == 0)
		{
			_nfa_block_destroy(block);
			return 0;
		}

		epsilon = _nfa_create_state();

		if(epsilon == NULL)
		{
			_nfa_block_destroy(block);
			return 0;
		}

		res = _nfa_add_epsilon_transition(block->end, epsilon);

		if(res == 0)
		{
			_nfa_block_destroy(block);
			_nfa_state_free(epsilon);
			return 0;
		}

		block->end = epsilon;

		epsilon = _nfa_create_state();

		if(epsilon == NULL)
		{
			_nfa_block_destroy(block);
			return 0;
		}

		res = _nfa_add_epsilon_transition(epsilon, block->start);

		if(res == 0)
		{
			_nfa_block_destroy(block);
			_nfa_state_free(epsilon);
			return 0;
		}

		block->start = epsilon;
	}

	if(operation != PLUS)
	{
		res = _nfa_add_epsilon_transition(block->start, block->end);

		if(res == 0)
		{
			_nfa_block_destroy(block);
			return 0;
		}
	}

	value.pointer = block;
	if(_stack_push(stack_, value, POINTER) == 0)
	{
		_nfa_block_destroy(block);
		return 0;
	}

	return 1;
}

/**
 * @brief create block for enumreation such as [ABCD]
 */
_nfa_block * _enumeration(char * input, unsigned length, unsigned * position)
{
	_Bool res;
	short symbol = _get_symbol(input, length, position);

	if(!_is_char(symbol))
	{
		errno = REGEX_INVALID;
		return NULL;
	}

	_nfa_block * block = _nfa_create_block(symbol);

	if(block == NULL) return _out_of_memory();

	while(*position < length)
	{
		symbol = _get_symbol(input, length, position);

		if(symbol == CLOSE_BRACKET) return block;
		else if(!_is_char(symbol)) break;

		res = _nfa_add_transition(block->start, symbol, block->end); //FIXME NULL possible

		if(res == 0) break;
	}

	_nfa_block_destroy(block);
	return _out_of_memory();
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

unsigned _precedence(stack * stack_, short current)
{
	if(_is_char(current) || current == DOT)
	{
		current = SYMBOL;
	}

	stack_item_value top = _stack_top_operation(stack_);

	return precedence_table[current - 300][top.number - 300];
}

_Bool _reduce(stack * stack_)
{
	_Bool res;
	stack_item_value value;
	short symbol = _stack_top_operation(stack_).number;

	switch(symbol)
	{
		case OR:
			res = _or(stack_); //FIXME NULL possible
			break;

		case CONCAT:
			res = _concat(stack_);
			break;

		case STAR:
		case PLUS:
		case QUESTION_MARK:
			res = _quantificator(stack_, symbol); //FIXME NULL possible
			break;
	}

	return res;
}

list * _dfa_epsilon_closure(list * nfa)
{
	list * closure = list_init();

	if(closure == NULL) return _out_of_memory();

	closure->head = nfa->head;
	closure->tail = nfa->tail;
	closure->size = nfa->size;

	bzero(nfa, sizeof(list));

	list_item * item = closure->head;
	list_item_value value;
	unsigned length;

	while(item != NULL)
	{
		length = ((_nfa_state *) item->value.pointer)->length_epsilon;

		for(unsigned i = 0; i < length; ++i)
		{
			value.pointer = ((_nfa_state *) item->value.pointer)->next_epsilon[i];
			if(list_append_unique(closure, value, POINTER) == 0) return list_destroy(closure), NULL;
		}

		item = item->next;
	}

	return closure;
}

_Bool _dfa_move(list * input_states, list * output_states, char character)
{
	list_item * item = input_states->head;
	list_item_value value;
	_nfa_state * state;
	void * position;

	while(item != NULL)
	{
		state = item->value.pointer;

		if((position = memchr(state->key, character, state->length)) != NULL)
		{
			value.pointer = state->next[(char *) position - state->key];
			if(list_append_unique(output_states, value, POINTER) == 0) return 0;
		}

		item = item->next;
	}

	return 1;
}

int _dfa_match_processed(list_item_value first, list_item_value second)
{
	list_item * first_item = ((_dfa_pair *)first.pointer)->nfa_states->head;
	list * second_list = ((list *)second.pointer);

	unsigned first_size = ((_dfa_pair *)first.pointer)->nfa_states->size;
	unsigned second_size = second_list->size;

	if(first_size != second_size) return 0;

	while(first_item != NULL)
	{
		if(!list_contains(second_list, first_item->value, POINTER)) return 0;

		first_item = first_item->next;
	}

	return 1;
}

/**
 * @brief Parse regular expression and generate DFA
 * @return [description]
 */
 regex_nfa * _nfa_construct(regex_pattern pattern)
{
	if(pattern.length == 0 || pattern.input == NULL)
	{
		errno = REGEX_EMPTY;
		return NULL;
	}

	short actual_symbol, current = START, prev = START;
	_Bool loop_again = 0;
	unsigned position = 0;
	unsigned precedence_res;
	unsigned length = pattern.length;
	char * input = pattern.input;
	regex_nfa * regex;
	_nfa_block * block;

	stack_item_value value;
	stack * stack_ = _stack_init();

	if(stack_ == NULL) return _out_of_memory();

	value.number = STACK_BOTTOM;

	if(_stack_push(stack_, value, NUMBER) == 0) return _nfa_clear(stack_);

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
					errno = REGEX_INVALID;
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
				if(_reduce(stack_) == 0) return _nfa_clear(stack_);
			}

			if(precedence_res == FAIL)
			{
				errno = REGEX_INVALID;
				return _nfa_clear(stack_);
			}
			else if(precedence_res == FIN) break;
			else if(precedence_res == EQUAL) // only in case current = CLOSE_PAREN && stack_top_operation == OPEN_PAREN
			{
				value = _stack_pop(stack_); // pop block
				_stack_pop(stack_); // pop OPEN_PAREN
				if(_stack_push(stack_, value, POINTER) == 0) return _nfa_block_destroy(value.pointer), _nfa_clear(stack_); // FIXME free value.pointer
				prev = CLOSE_PAREN;
			}

			switch(current)
			{
				case CLOSE_PAREN:
					break;

				case DOT:
					prev = DOT;
					value.pointer = _nfa_create_dot_block();
					if(value.pointer == NULL) return _nfa_clear(stack_);
					if(_stack_push(stack_, value, POINTER) == 0) return _nfa_clear(stack_); // FIXME free value.pointer
					break;

				case OPEN_BRACKET:
					value.pointer = _enumeration(input, length, &position);
					if(value.pointer == NULL) return _nfa_clear(stack_);
					if(_stack_push(stack_, value, POINTER) == 0) return _nfa_block_destroy(value.pointer), _nfa_clear(stack_); // FIXME free value.pointer
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
					if(_stack_push(stack_, value, NUMBER) == 0) return _nfa_clear(stack_); // FIXME free value.pointer
					break;

				default:
					value.pointer = _nfa_create_block(current);
					if(value.pointer == NULL) return _nfa_clear(stack_);
					if(_stack_push(stack_, value, POINTER) == 0) return _nfa_block_destroy(value.pointer), _nfa_clear(stack_); // FIXME free value.pointer
					prev = SYMBOL;
					break;
			}
		} while(loop_again);
	}

	while(_stack_top_operation(stack_).number != STACK_BOTTOM)
	{
		if(_reduce(stack_) == 0) return _nfa_clear(stack_);
	}

	assert(_stack_size(stack_) == 2);
	assert(_stack_top_operation(stack_).number == STACK_BOTTOM);

	block = _stack_pop(stack_).pointer;
	block->end->id = pattern.id;
	regex = block->start;
	free(block);

	_stack_destroy(stack_);

	return regex;
}


regex_dfa * _nfa_convert(regex_nfa * root)
{
	list_item_value value;
	_dfa_pair * pair;
	_dfa_pair * new_pair;
	list_item_value * existing_pair;
	_dfa_state * dfa_root;
	list * closure_states = NULL;
	list * nfa_list = NULL;
	list * move_states = NULL;
	list * pairs_created = NULL;
	stack * pairs_unprocessed = NULL;

	if((nfa_list = _stack_init()) == NULL) return _DFA_CLEAR(1);
	if((move_states = _stack_init()) == NULL) return _DFA_CLEAR(1);
	if((pairs_created = _stack_init()) == NULL) return _DFA_CLEAR(1);
	if((pairs_unprocessed = _stack_init()) == NULL) return _DFA_CLEAR(1);

	value.pointer = root;
	if(list_append_front(nfa_list, value, POINTER) == 0) return _DFA_CLEAR(1);
	if((closure_states = _dfa_epsilon_closure(nfa_list)) == NULL) return _DFA_CLEAR(1);

	list_destroy(nfa_list);
	nfa_list = NULL;

	if((pair = _dfa_create_pair(closure_states)) == NULL) return _DFA_CLEAR(1);

	dfa_root = pair->dfa_state;

	value.pointer = pair;
	if((_stack_push(pairs_unprocessed, value, POINTER)) == 0) return _DFA_CLEAR(1);
	if((list_append_back(pairs_created, value, POINTER)) == 0) return _DFA_CLEAR(1);

	while(!_stack_empty(pairs_unprocessed))
	{
		pair = _stack_pop(pairs_unprocessed).pointer;

		for(int i = CHAR_MIN; i <= CHAR_MAX; ++i)
		{
			if(_dfa_move(pair->nfa_states, move_states, i) == 0) return _DFA_CLEAR(1);
			if((closure_states = _dfa_epsilon_closure(move_states)) == NULL) return _DFA_CLEAR(1);

			if(closure_states->size == 0)
			{
				list_destroy(closure_states);
				closure_states = NULL;
				continue;
			}

			value.pointer = closure_states;
			existing_pair = list_find(pairs_created, value, _dfa_match_processed);

			if(existing_pair == NULL)
			{
				new_pair = _dfa_create_pair(closure_states);
				if(new_pair == NULL) return _DFA_CLEAR(1);
				value.pointer = new_pair;

				if(_stack_push(pairs_unprocessed, value, POINTER) == 0) return _dfa_pair_free(new_pair), _DFA_CLEAR(1);
				if(list_append_back(pairs_created, value, POINTER) == 0) return _DFA_CLEAR(1);
				if(_dfa_add_transition(pair->dfa_state, i, new_pair->dfa_state) == 0) return _DFA_CLEAR(1);

			}
			else
			{
				list_destroy(closure_states);
				closure_states = NULL;
				if(_dfa_add_transition(pair->dfa_state, i, ((_dfa_pair *) existing_pair->pointer)->dfa_state) == 0) return _DFA_CLEAR(1);
			}
		}
	}

	_DFA_CLEAR(0);
	return dfa_root;

}

regex_nfa * regex_construct_nfa(regex_pattern * patterns, unsigned count)
{
	if(count <= 0)
	{
		errno = REGEX_EMPTY;
		return NULL;
	}

	unsigned i = 0;
	_Bool res;
	regex_nfa * regex = NULL;
	regex_nfa * root = NULL;

	for(i = 0; i < count; ++i)
	{
		if(patterns[i].id == 0)
		{
			errno = REGEX_INVALID_ID;
			regex_destroy_nfa(root);
		}

		regex = _nfa_construct(patterns[i]);

		if(regex == NULL) return _out_of_memory();

		if(i == 0) root = regex;
		else
		{
			res = _nfa_add_epsilon_transition(root, regex);

			if(res == 0)
			{
				regex_destroy_nfa(root);
				regex_destroy_nfa(regex);
				return _out_of_memory();
			}
		}
	}

	return root;
}

regex_dfa * regex_construct_dfa(regex_pattern * patterns, unsigned count)
{
	regex_nfa * nfa = regex_construct_nfa(patterns, count);

	if(nfa == NULL) return _out_of_memory();

	return _nfa_convert(nfa);
}

void regex_destroy_nfa(regex_nfa * root)
{
	if(root == NULL) return;

	_nfa_destroy(root);
}

void regex_destroy_dfa(regex_dfa * root)
{
	if(root == NULL) return;

	_dfa_destroy(root);
}

int regex_match_nfa(regex_nfa * regex, char * input, unsigned length)
{
	queue * start = _queue_init(); //FIXME NULL possible
	queue * end = _queue_init(); //FIXME NULL possible
	queue * swap_pointer;
	queue_item_value value;
	_nfa_state * state;
	unsigned position = 0;
	int result = NOT_MATCH;
	void * char_position;

	value.pointer = regex;
	_queue_insert(start, value, POINTER); //FIXME NULL possible


	while(position < length)
	{
		while(!_queue_empty(start))
		{
			state = _queue_pop_front(start).pointer;

			for(unsigned i = 0; i < state->length_epsilon; ++i)
			{
				value.pointer = state->next_epsilon[i];
				_queue_insert_unique(start, value, POINTER); //FIXME NULL possible
			}

			if((char_position = memchr(state->key, input[position], state->length))) // OR final state
			{
				value.pointer = state->next[char_position - (void *) state->key];
				_queue_insert_unique(end, value, POINTER); //FIXME NULL possible
			}
		}

		value.pointer = regex;
		_queue_insert_unique(end, value, POINTER); //FIXME NULL possible

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

int regex_match_dfa(regex_dfa * regex, char * input, unsigned length)
{
	void * char_position;
	_dfa_state * state = regex;

	for(unsigned position = 0; position < length; ++position)
	{
		if((char_position = memchr(state->key, input[position], state->length)))
		{
			state = state->next[char_position - (void *) state->key];
		}
		else
		{
			state = regex;
		}
	}

	return state->id != ID_NONE ? state->id : NOT_MATCH;
}
