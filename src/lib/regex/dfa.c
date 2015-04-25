#include "dfa.h"
#include "../general/stack.h"

void * _out_of_memory()
{
	errno = FASTNET_OUT_OF_MEMORY;
	return NULL;
}

/**
 * @brief Handle escaped character
 * @param input
 * @param length length if <input>
 * @param position posiiton in <input>
 * @return character
 */
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
 * @brief Process regex expression and return each symbol
 * @param input
 * @param length length of <input>
 * @param position position in <input>
 * @return symbol
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

/**
 * @brief Add transition into <next> to <next_pointer> and save transition symbol to <key>
 * @param key symbols
 * @param next next pointers
 * @param length length of <key> & <next>
 * @param symbol transition symbol
 * @param next_pointer destination of transition
 * @return success/failure
 */
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

/**
 * @brief Add epsilon transtion from state to next
 * @param state start of transition
 * @param next destination of transition
 * @return success/failure
 */
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

/**
 * @brief Allocate memory for new nfa state
 * @return pointer to new state
 */
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

/**
 * @brief Allocate memory for new dfa state
 * @return pointer to new state
 */
_dfa_state * _dfa_create_state()
{
	_dfa_state * state = malloc(sizeof(_dfa_state));

	if(state == NULL) return _out_of_memory();

	state->length = 0;
	state->next = NULL;
	state->key = NULL;
	state->id = ID_NONE;

	return state;
}

/**
 * @brief Free memory of nfa state
 */
void _nfa_state_free(_nfa_state * state)
{
	free(state->key);
	free(state->next);
	free(state->next_epsilon);
	free(state);
}

/**
 * @brief Free memory of dfa state
 */
void _dfa_state_free(_dfa_state * state)
{
	assert(state != NULL);

	free(state->key);
	free(state->next);
	free(state);
}

/**
 * @brief Destroy whole nfa
 * @param already_free already free states to avoid double free
 * @param state starting state
 */
void _nfa_state_destroy(stack * already_freed, _nfa_state * state)
{
	stack_item_value value;
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

	stack_push_unique(already_freed, value, POINTER);

	for(unsigned i = 0; i < length; ++i)
	{
		value.pointer = next[i];
		if(stack_contains(already_freed, value, POINTER)) continue;
		_nfa_state_destroy(already_freed, value.pointer);
	}

	free(next);

	for(unsigned i = 0; i < length_epsilon; ++i)
	{
		value.pointer = next_epsilon[i];
		if(stack_contains(already_freed, value, POINTER)) continue;
		_nfa_state_destroy(already_freed, value.pointer);
	}

	free(next_epsilon);
}

/**
 * @brief Destroy whole dfa
 * @param already_free already free states to avoid double free
 * @param state starting state
 */
void _dfa_state_destroy(stack * already_freed, _dfa_state * state)
{
	stack_item_value value;
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

	stack_push(already_freed, value, POINTER);

	for(unsigned i = 0; i < length; ++i)
	{
		value.pointer = next[i];
		if(stack_contains(already_freed, value, POINTER)) continue;
		_dfa_state_destroy(already_freed, value.pointer);
	}

	free(next);
}

/**
 * @brief Destroy whole dfa
 * @param state root state
 */
void _dfa_destroy(_dfa_state * state)
{
	stack already_freed;

	already_freed.data = NULL;
	already_freed.size = 0;
	already_freed.top = -1;

	_dfa_state_destroy(&already_freed, state);
	free(already_freed.data);
}

/**
 * @brief Destroy whole nfa
 * @param state root state
 */
void _nfa_destroy(_nfa_state * state)
{
	stack already_freed;

	already_freed.data = NULL;
	already_freed.size = 0;
	already_freed.top = -1;

	_nfa_state_destroy(&already_freed, state);
	free(already_freed.data);
}

/**
 * @brief Create block consisting of 2 states with <symbol> transition between them
 * @param symbol
 * @return pointer to block
 */
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

	res = _nfa_add_transition(block->start, symbol, block->end);

	if(res == 0)
	{
		_nfa_state_free(block->start);
		_nfa_state_free(block->end);
		free(block);

		return _out_of_memory();
	}

	return block;
}

/**
 * @brief Create block consisting of 2 states with transition for every symbol possible
 * @return pointer to block
 */
_nfa_block * _nfa_create_dot_block()
{
	_Bool res;
	_nfa_block * block = _nfa_create_block(0);

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

/**
 * @brief Create dfa pair consisting of new dfa state and <nfa_states> from which dfa state can be reached
 * @param nfa_states list of nfa states
 * @return pointer to pair
 */
_dfa_pair * _dfa_create_pair(stack ** nfa_states)
{
	unsigned char id;

	stack * tmp = stack_init();
	if(tmp == NULL) return _out_of_memory();

	_dfa_pair * pair = malloc(sizeof(_dfa_pair));
	if(pair == NULL) return stack_destroy(tmp), _out_of_memory();

	pair->dfa_state = _dfa_create_state();
	if(pair->dfa_state == NULL)
	{
		free(pair);
		return stack_destroy(tmp), _out_of_memory();
	}

	pair->nfa_states = *nfa_states;
	*nfa_states = tmp;

	for(int i = 0; i <= pair->nfa_states->top; ++i)
	{
		id = ((_nfa_state *) pair->nfa_states->data[i].value.pointer)->id;
		if(id != ID_NONE)
		{
			pair->dfa_state->id = id;
			break;
		}
	}

	return pair;
}

/**
 * @brief Destroy nfa block
 * @param block block to destroy
 */
void _nfa_block_destroy(void * block)
{
	_nfa_state * start_state = ((_nfa_block *) block)->start;

	free(block);

	_nfa_destroy(start_state);
}

/**
 * @brief Free memory allocated for dfa pair
 * @param pair pair to be freed
 */
void _dfa_pair_free(void * pair)
{
	stack_destroy(((_dfa_pair *) pair)->nfa_states);
	free(pair);
}

/**
 * @brief Destroy dfa pair
 * @param pair pair to be destroyed
 */
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
	stack_free_pointers(stack_, _nfa_block_destroy);
	stack_destroy(stack_);

	return NULL;
}

/**
 * @brief Determinization failed, clear all data and return null to signalize error
 * @param root
 * @param nfa_states
 * @param move_states
 * @param pairs_created
 * @param closure_states
 * @param pairs_unprocessed
 * @param free_dfa_states
 * @return NULL
 */
void * _dfa_clear(_nfa_state * root, stack * nfa_states, stack * move_states, stack * pairs_created, stack * closure_states, stack * pairs_unprocessed, _Bool free_dfa_states)
{
	regex_destroy_nfa(root);

	stack_destroy(nfa_states);
	stack_destroy(move_states);
	stack_destroy(closure_states);

	if(free_dfa_states) stack_free_pointers(pairs_created, _dfa_pair_destroy);
	else stack_free_pointers(pairs_created, _dfa_pair_free);
	stack_destroy(pairs_created);
	stack_free_pointers(pairs_unprocessed, _dfa_pair_destroy);
	stack_destroy(pairs_unprocessed);

	return NULL;
}

/**
 * @brief Concatenate two construction blocks
 * @param stack_ of construction blocks
 */
_Bool _concat(stack * stack_)
{
	assert(stack_ != NULL);
	assert(stack_size(stack_) >= 3);

	stack_item_value value;
	_nfa_block * first;
	_nfa_block * second;

	second = stack_pop(stack_).pointer;
	value = stack_pop(stack_);
	assert(value.number == CONCAT);
	first = stack_pop(stack_).pointer;

	// _nfa_add_epsilon_transition(first->end, second->start);

	memcpy(first->end, second->start, sizeof(_nfa_state));
	first->end = second->end;

	free(second->start);
	free(second);

	value.pointer = first;
	if(stack_push(stack_, value, POINTER) == 0)
	{
		_nfa_block_destroy(value.pointer);
		return 0;
	}

	return 1;
}

/**
 * @brief Create OR block
 * @param stack_ stack of blocks
 * @return success/failure
 */
_Bool _or(stack * stack_)
{
	assert(stack_ != NULL);
	assert(stack_size(stack_) >= 3);

	stack_item_value value;
	_nfa_block * first;
	_nfa_block * second;
	_Bool res;

	second = stack_pop(stack_).pointer;
	value = stack_pop(stack_);
	assert(value.number == OR);
	first = stack_pop(stack_).pointer;

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

	if(stack_push(stack_, value, POINTER) == 0)
	{
		_nfa_block_destroy(first);
		return 0;
	}

	return 1;
}

/**
 * @brief Construct quantificator
 * @param stack_ stack of construction blocks
 * @param operation operation to be constructed
 * @return success/failure
 */
_Bool _quantificator(stack * stack_, short operation)
{
	assert(stack_ != NULL);
	assert(stack_size(stack_) >= 2);

	stack_item_value value;
	_nfa_block * block;
	_nfa_state * epsilon;
	_Bool res;

	value = stack_pop(stack_);
	assert(value.number == operation);
	block = stack_pop(stack_).pointer;

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
	if(stack_push(stack_, value, POINTER) == 0)
	{
		_nfa_block_destroy(block);
		return 0;
	}

	return 1;
}

/**
 * @brief Create block for enumreation such as [ABCD]
 * @param input input text
 * @param length length of <input>
 * @param position position in <input>
 * @return pointer to constructed block
 */
_nfa_block * _enumeration(char * input, unsigned length, unsigned * position)
{
	_Bool res;
	short symbol = _get_symbol(input, length, position);

	if(!_is_char(symbol))
	{
		errno = FASTNET_REGEX_INVALID;
		return NULL;
	}

	_nfa_block * block = _nfa_create_block(symbol);

	if(block == NULL) return _out_of_memory();

	while(*position < length)
	{
		symbol = _get_symbol(input, length, position);

		if(symbol == CLOSE_BRACKET) return block;
		else if(!_is_char(symbol)) break;

		res = _nfa_add_transition(block->start, symbol, block->end);

		if(res == 0) break;
	}

	_nfa_block_destroy(block);
	return _out_of_memory();
}

/**
 * @brief Check if two symbols can be placed one after another
 * @param current current symbol
 * @param prev previous symbol
 * @return yes/no
 */
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

/**
 * @brief Decide which operation has higher priority
 * @param stack_ stack of symbols
 * @param current current symbol
 * @return SHIFT/REDUCE/EQUAL/FIN
 */
unsigned _precedence(stack * stack_, short current)
{
	if(_is_char(current) || current == DOT)
	{
		current = SYMBOL;
	}

	stack_item_value top = stack_top_type(stack_, NUMBER);

	return precedence_table[current - 300][top.number - 300];
}

/**
 * @brief Reduce operation which is on top of <stack_>
 * @param stack_ stack of operations
 * @return success/failure
 */
_Bool _reduce(stack * stack_)
{
	_Bool res;
	short symbol = stack_top_type(stack_, NUMBER).number;

	switch(symbol)
	{
		case OR:
			res = _or(stack_);
			break;

		case CONCAT:
			res = _concat(stack_);
			break;

		case STAR:
		case PLUS:
		case QUESTION_MARK:
			res = _quantificator(stack_, symbol);
			break;
	}

	return res;
}

/**
 * @brief Perform epsilon closure
 * @param nfa stack of nfa states
 * @param closure[out] stack of all epsilon closures for <nfa>
 * @return success/failure
 */
_Bool _dfa_epsilon_closure(stack * nfa, stack * closure)
{
	stack swap_stack;

	memcpy(&swap_stack, nfa, sizeof(stack));
	memcpy(nfa, closure, sizeof(stack));
	memcpy(closure, &swap_stack, sizeof(stack));

	stack_item_value value;
	stack_item * item;
	unsigned length;

	for(int i = 0; i <= closure->top; ++i)
	{
		item = closure->data[i].value.pointer;
		length = ((_nfa_state *) item)->length_epsilon;

		for(unsigned i = 0; i < length; ++i)
		{
			value.pointer = ((_nfa_state *) item)->next_epsilon[i];
			if(stack_push_unique(closure, value, POINTER) == 0) return 0;
		}
	}

	return 1;
}

/**
 * @brief Perform move function on <input_states> with <character>
 * @param input_states
 * @param output_states
 * @param character
 * @return success/failure
 */
_Bool _dfa_move(stack * input_states, stack * output_states, char character)
{
	stack_item_value value;
	_nfa_state * state;
	void * position;

	for(int i = 0; i <= input_states->top; ++i)
	{
		state = input_states->data[i].value.pointer;

		if((position = memchr(state->key, character, state->length)) != NULL)
		{
			value.pointer = state->next[(char *) position - state->key];
			if(stack_push_unique(output_states, value, POINTER) == 0) return 0;
		}
	}

	return 1;
}

/**
 * @brief Check if two stack are equal
 * @param first
 * @param second
 * @return success/failure
 */
_Bool _dfa_match_processed(stack_item_value first, stack_item_value second)
{
	stack * first_stack = ((_dfa_pair *)first.pointer)->nfa_states;
	stack * second_stack = second.pointer;

	if(first_stack->top != second_stack->top) return 0;

	for(int i = 0; i <= first_stack->top; ++i)
	{
		if(!stack_contains(second.pointer, first_stack->data[i].value, POINTER)) return 0;
	}

	return 1;
}

/**
 * @brief Parse regular expression and generate NFA
 * @return pointer to nfa
 */
 regex_nfa * _nfa_construct(regex_pattern pattern)
{
	if(pattern.length == 0 || pattern.input == NULL)
	{
		errno = FASTNET_REGEX_EMPTY;
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
	stack * stack_ = stack_init();

	if(stack_ == NULL) return _out_of_memory();

	value.number = STACK_BOTTOM;

	if(stack_push(stack_, value, NUMBER) == 0) return _nfa_clear(stack_);

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
					errno = FASTNET_REGEX_INVALID;
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
				errno = FASTNET_REGEX_INVALID;
				return _nfa_clear(stack_);
			}
			else if(precedence_res == FIN) break;
			else if(precedence_res == EQUAL) // only in case current = CLOSE_PAREN && stack_top_operation == OPEN_PAREN
			{
				value = stack_pop(stack_); // pop block
				stack_pop(stack_); // pop OPEN_PAREN
				if(stack_push(stack_, value, POINTER) == 0) return _nfa_block_destroy(value.pointer), _nfa_clear(stack_);
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
					if(stack_push(stack_, value, POINTER) == 0) return _nfa_block_destroy(value.pointer), _nfa_clear(stack_);
					break;

				case OPEN_BRACKET:
					value.pointer = _enumeration(input, length, &position);
					if(value.pointer == NULL) return _nfa_clear(stack_);
					if(stack_push(stack_, value, POINTER) == 0) return _nfa_block_destroy(value.pointer), _nfa_clear(stack_);
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
					if(stack_push(stack_, value, NUMBER) == 0) return _nfa_clear(stack_);
					break;

				default:
					value.pointer = _nfa_create_block(current);
					if(value.pointer == NULL) return _nfa_clear(stack_);
					if(stack_push(stack_, value, POINTER) == 0) return _nfa_block_destroy(value.pointer), _nfa_clear(stack_);
					prev = SYMBOL;
					break;
			}
		} while(loop_again);
	}

	while(stack_top_type(stack_, NUMBER).number != STACK_BOTTOM)
	{
		if(_reduce(stack_) == 0) return _nfa_clear(stack_);
	}

	assert(stack_size(stack_) == 2);
	assert(stack_top_type(stack_, NUMBER).number == STACK_BOTTOM);

	block = stack_pop(stack_).pointer;
	block->end->id = pattern.id;
	regex = block->start;
	free(block);

	stack_destroy(stack_);

	return regex;
}

/**
 * @brief Conver nfa to dfa
 * @param root start of nfa
 * @return pointer to dfa
 */
regex_dfa * _nfa_convert(regex_nfa * root)
{
	stack_item_value value;
	_dfa_pair * pair;
	_dfa_pair * new_pair;
	stack_item_value * existing_pair;
	_dfa_state * dfa_root;
	stack * closure_states = NULL;
	stack * nfa_list = NULL;
	stack * move_states = NULL;
	stack * pairs_created = NULL;
	stack * pairs_unprocessed = NULL;

	if((nfa_list = stack_init()) == NULL) return _DFA_CLEAR(1);
	if((move_states = stack_init()) == NULL) return _DFA_CLEAR(1);
	if((pairs_created = stack_init()) == NULL) return _DFA_CLEAR(1);
	if((pairs_unprocessed = stack_init()) == NULL) return _DFA_CLEAR(1);
	if((closure_states = stack_init()) == NULL) return _DFA_CLEAR(1);

	value.pointer = root;
	if(stack_push(nfa_list, value, POINTER) == 0) return _DFA_CLEAR(1);
	if(_dfa_epsilon_closure(nfa_list, closure_states) == 0) return _DFA_CLEAR(1);

	stack_destroy(nfa_list);
	nfa_list = NULL;

	if((pair = _dfa_create_pair(&closure_states)) == NULL) return _DFA_CLEAR(1);

	dfa_root = pair->dfa_state;

	value.pointer = pair;
	if((stack_push(pairs_unprocessed, value, POINTER)) == 0) return _DFA_CLEAR(1);
	if((stack_push(pairs_created, value, POINTER)) == 0) return _DFA_CLEAR(1);

	while(!stack_empty(pairs_unprocessed))
	{
		pair = stack_pop(pairs_unprocessed).pointer;

		for(int i = CHAR_MIN; i <= CHAR_MAX; ++i)
		{
			if(_dfa_move(pair->nfa_states, move_states, i) == 0) return _DFA_CLEAR(1);
			if(_dfa_epsilon_closure(move_states, closure_states) == 0) return _DFA_CLEAR(1);

			if(stack_empty(closure_states))
			{
				continue;
			}

			value.pointer = closure_states;
			existing_pair = stack_find(pairs_created, value, _dfa_match_processed);

			if(existing_pair == NULL)
			{
				new_pair = _dfa_create_pair(&closure_states);
				if(new_pair == NULL) return _DFA_CLEAR(1);
				value.pointer = new_pair;

				if(stack_push(pairs_unprocessed, value, POINTER) == 0) return _dfa_pair_free(new_pair), _DFA_CLEAR(1);
				if(stack_push(pairs_created, value, POINTER) == 0) return _DFA_CLEAR(1);
				if(_dfa_add_transition(pair->dfa_state, i, new_pair->dfa_state) == 0) return _DFA_CLEAR(1);

			}
			else
			{
				closure_states->top = -1;
				if(_dfa_add_transition(pair->dfa_state, i, ((_dfa_pair *) existing_pair->pointer)->dfa_state) == 0) return _DFA_CLEAR(1);
			}
		}
	}

	_DFA_CLEAR(0);
	return dfa_root;

}

/**
 * @brief Construct NFA
 * @param patterns array of patterns
 * @param count size of <patterns>
 * @return pointer to nfa
 */
regex_nfa * regex_construct_nfa(regex_pattern * patterns, unsigned count)
{
	if(count <= 0)
	{
		errno = FASTNET_REGEX_EMPTY;
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
			errno = FASTNET_REGEX_INVALID_ID;
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

/**
 * @brief Construct DFA
 * @param patterns array of patterns
 * @param count size of <patterns>
 * @return pointer to dfa
 */
regex_dfa * regex_construct_dfa(regex_pattern * patterns, unsigned count)
{
	regex_nfa * nfa = regex_construct_nfa(patterns, count);

	if(nfa == NULL) return _out_of_memory();

	return _nfa_convert(nfa);
}

/**
 * @brief Destroy NFA
 * @param root start state of nfa
 */
void regex_destroy_nfa(regex_nfa * root)
{
	if(root == NULL) return;

	_nfa_destroy(root);
}

/**
 * @brief Destroy DFA
 * @param root start state of dfa
 */
void regex_destroy_dfa(regex_dfa * root)
{
	if(root == NULL) return;

	_dfa_destroy(root);
}

/**
 * @brief Match <regex> against <input>
 * @param regex
 * @param input
 * @param length length of <input>
 * @return matched pattern
 */
int regex_match_nfa(regex_nfa * regex, char * input, unsigned length)
{
	stack * start = NULL;
	stack * end = NULL;
	stack * swap_pointer;
	stack_item_value value;
	_nfa_state * state;
	unsigned position = 0;
	int result = NOT_MATCH;
	void * char_position;

	if((start = stack_init()) == NULL) return errno = FASTNET_OUT_OF_MEMORY, NOT_MATCH;
	if((end = stack_init()) == NULL) return errno = FASTNET_OUT_OF_MEMORY, stack_destroy(start), NOT_MATCH;

	value.pointer = regex;
	if(stack_push(start, value, POINTER) == 0) return errno = FASTNET_OUT_OF_MEMORY, stack_destroy(start), stack_destroy(end), NOT_MATCH;

	while(position < length)
	{
		while(!stack_empty(start))
		{
			state = stack_pop(start).pointer;

			if(state->id != ID_NONE)
			{
				result = state->id;
				goto NFA_END;
			}

			for(unsigned i = 0; i < state->length_epsilon; ++i)
			{
				value.pointer = state->next_epsilon[i];

				if(stack_push_unique(start, value, POINTER) == 0) return errno = FASTNET_OUT_OF_MEMORY, stack_destroy(start), stack_destroy(end), NOT_MATCH;
			}

			if((char_position = memchr(state->key, input[position], state->length))) // OR final state
			{
				value.pointer = state->next[char_position - (void *) state->key];
				if(stack_push_unique(end, value, POINTER) == 0) return errno = FASTNET_OUT_OF_MEMORY, stack_destroy(start), stack_destroy(end), NOT_MATCH;
			}
		}

		if(stack_empty(end))
		{
			value.pointer = regex;
			if(stack_push_unique(end, value, POINTER) == 0) return errno = FASTNET_OUT_OF_MEMORY, stack_destroy(start), stack_destroy(end), NOT_MATCH;
		}

		swap_pointer = end;
		end = start;
		start = swap_pointer;

 		++position;
	}

	while(!stack_empty(start))
	{
		state = stack_pop(start).pointer;

		if(state->id != ID_NONE)
		{
			result = state->id;
			break;
		}

		for(unsigned i = 0; i < state->length_epsilon; ++i)
		{
			value.pointer = state->next_epsilon[i];
			if(stack_push_unique(start, value, POINTER) == 0) return errno = FASTNET_OUT_OF_MEMORY, stack_destroy(start), stack_destroy(end), NOT_MATCH;
		}
	}
NFA_END:
	stack_destroy(start);
	stack_destroy(end);

	return result;

}

/**
 * @brief Match <regex> against <input>
 * @param regex
 * @param input
 * @param length length of <input>
 * @return matched pattern
 */
int regex_match_dfa(regex_dfa * regex, char * input, unsigned length)
{
	void * char_position;
	_dfa_state * state = regex;

	if(state->id != ID_NONE) return state->id;

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

		if(state->id != ID_NONE) return state->id;
	}

	return NOT_MATCH;
}
