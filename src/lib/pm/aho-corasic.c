#include "aho-corasic.h"

void _ac_remove_rule(_ac_state * state, _AC_RULE rule)
{
	for(unsigned i = 0; i < state->additional_size; ++i)
	{
		if(state->additional_rule[i] == rule)
		{
			for(unsigned j = i; j < state->additional_size - 1; ++j)
			{
				state->additional_rule[j] = state->additional_rule[j + 1];
			}

			--state->additional_size;
			state->additional_rule = realloc(state->additional_rule, sizeof(_AC_RULE) * state->additional_size);

			return;
		}
	}
}

int _ac_goto(_ac_state * state, char character)
{
	char * pos = memchr(state->key, character, state->path_count);

	if(pos == NULL) return FAIL;

	return pos - state->key;
}

bool _ac_queue_empty(pm_root * root)
{
	return root->queue->head == NULL;
}

void _ac_queue_insert(pm_root * root, _ac_state * state)
{
	_ac_queue_item * item = malloc(sizeof(_ac_queue_item));

	item->state = state;
	item->next = NULL;

	if(_ac_queue_empty(root))
	{
		root->queue->head = root->queue->tail = item;
	}
	else
	{
		root->queue->tail->next = item;
		root->queue->tail = item;
	}
}

_ac_state * _ac_queue_front(pm_root * root)
{
	_ac_queue_item * item = root->queue->head;
	_ac_state * state = item->state;

	root->queue->head = item->next;

	free(item);

	return state;
}

void _ac_append_rule(_ac_state * state, _AC_RULE rule)
{
	for(unsigned i = 0; i < state->additional_size; ++i)
	{
		if(state->additional_rule[i] == rule) return;
	}

	++state->additional_size;
	state->additional_rule = realloc(state->additional_rule, state->additional_size * sizeof(_AC_RULE));
	state->additional_rule[state->additional_size - 1] = rule;
}


void _ac_construct_failure(pm_root * root, _AC_RULE removed_rule)
{
	_ac_state * state = root->state;
	_ac_state * s, * r;
	int goto_pos;

	// start constructing, go throught every direct follower of root state = depth 1
	for(unsigned i = 0; i < state->path_count; ++i)
	{
		// depth 1, failure for every state at this depth is root state
		state->next[i]->failure = root->state;

		// "every path in root state is defined" => skip those with no real follower
		if(state->next[i] == root->state) continue;

		_ac_queue_insert(root, state->next[i]);
	}


	while(!_ac_queue_empty(root))
	{
		r = _ac_queue_front(root);

		unsigned length = r->path_count;

		// for every follower
		for(unsigned i = 0; i < length; ++i)
		{
			s = r->next[i];
			_ac_queue_insert(root, s);
			state = r->failure;

			// find failure path
			while((goto_pos = _ac_goto(state, r->key[i])) == FAIL)
			{
				state = state->failure;
			}

			s->failure = state->next[goto_pos];

			_ac_remove_rule(s, removed_rule);
			_ac_append_rule(s, s->failure->rule);
			// copy all rules from failure state to this state
			for(unsigned j = 0; j < s->failure->additional_size; ++j)
			{
				_ac_append_rule(s, s->failure->additional_rule[j]);
			}

		}
	}
}



/**
 * @brief Construct node
 * @return pointer to node
 */
_ac_state * _ac_create()
{
	_ac_state * state = (_ac_state *) malloc(sizeof(_ac_state));
	state->rule = NONE;
	state->additional_rule = NULL;
	state->additional_size = 0;
	//TODO nějak zrušit tenhle dočasný malloc
	state->key = NULL;
	state->path_count = 0;
	state->next = NULL;
	state->failure = NULL;

	return state;
}


void _ac_add_match(pm_match ** match, _AC_RULE matched_rule)
{
	if((*match)->size == (*match)->count)
	{
		(*match)->size <<= 1;
		(*match)->rule = (_AC_RULE *) realloc((*match)->rule, (*match)->size * sizeof(_AC_RULE));
	}

	(*match)->rule[(*match)->count++] = matched_rule;
}

/*
 * @brief Append node to parent and resize array of next pointers & keys
 * @param node
 * @param parent
 * @param character
 */
void _ac_append(pm_root * root, _ac_state * state, _ac_state * parent, char character)
{
	if(parent == root->state)
	{
		parent->next[ (unsigned char) character] = state;
		return;
	}
	// resize arrays
	parent->path_count++;
	parent->key = realloc(parent->key, parent->path_count);
	parent->next = realloc(parent->next, parent->path_count * sizeof(_ac_state *));
	// and save new values
	parent->key[parent->path_count - 1] = character;
	parent->next[parent->path_count - 1] = state;
}

/*
 * @brief Go trought tree until there is a path to take
 * @param node start point
 * @param text source text for which path is searched
 * @param length[out] length of matched path
 * @return last matched node
 */
_ac_state * _ac_longest_match(pm_root * root, char * text, unsigned size, size_t * length)
{
	int goto_pos;
	_ac_state * state = root->state;
	*length = 0;
	unsigned i = 0;

	while(i < size && (goto_pos = _ac_goto(state, text[i])) != FAIL)
	{
		state = state->next[goto_pos];

		if(state == root->state) break;
		++i;
		++*length;

	}

	return state;
}

void _ac_free(_ac_state * state)
{
	free(state->key);
	free(state->next);
	free(state->additional_rule);
	free(state);
}

/**
 * @brief Remove branch from prev
 */
void _ac_remove(pm_root * root, _ac_state * prev, char character, _AC_RULE * removed_rule)
{
	_ac_state * state;
	char * pos;
	size_t key_length;

	key_length = prev->path_count;
	--prev->path_count;
	pos = memchr(prev->key, character, key_length);
	state = prev->next[pos - prev->key];

	if(prev == root->state)
	{
		prev->next[pos - prev->key] = root->state;
	}
	else
	{
		for(unsigned i = pos - prev->key; i < key_length - 1; ++i)
		{
			prev->key[i] = prev->key[i + 1];
			prev->next[i] = prev->next[i + 1];
		}
	}


	while(state->next != NULL)
	{
		prev = state;
		state = state->next[0];
		_ac_free(prev);
	}

	*removed_rule = state->rule;
	_ac_free(state);
}

void _ac_destroy(_ac_state * state)
{
	for(unsigned i = 0; i < state->path_count; ++i)
	{
		_ac_destroy(state->next[i]);
	}

	_ac_free(state);
}

/*
 * @brief Init data structures
 * @return root
 */
pm_root * init()
{
	pm_root * root;

	root = malloc(sizeof(pm_root));

	root->state = _ac_create();
	root->state->path_count = 255;
	root->state->key = malloc(256);
	root->state->next = malloc(256 * sizeof(_ac_state *));

	for(unsigned i = 0; i < 256; ++i)
	{
		root->state->key[i] = i;
		root->state->next[i] = root->state;
	}

	root->match = malloc(sizeof(pm_match));
	root->match->rule = malloc(sizeof(_AC_RULE) * 10);
	root->match->count = 0;
	root->match->size = 10;

	root->queue = malloc(sizeof(_ac_queue));
	root->queue->head = NULL;
	root->queue->tail = NULL;

	return root;
}

pm_match * match(pm_root * root, char * text, unsigned length)
{
	_ac_state * state = root->state;
	int goto_pos;

	for(size_t pos = 0; pos < length; ++pos)
	{
		while((goto_pos = _ac_goto(state, text[pos])) == FAIL) state = state->failure;
		state = state->next[goto_pos];

		if(state->rule != NONE || state->additional_size > 0)
		{
			_ac_add_match(&(root->match), state->rule);

			for(unsigned i = 0; i < state->additional_size; ++i)
			{
				_ac_add_match(&(root->match), state->additional_rule[i]);
			}

			root->match->position = pos + 1;
			root->match->state = state;
			root->match->text = text;
			root->match->length = length;

			return root->match;
		}
	}

	return NULL;
}

pm_match * match_next(pm_root * root)
{
	_ac_state * state = root->match->state;
	int goto_pos;
	char * text = root->match->text;
	root->match->count = 0;

	for(size_t pos = root->match->position; pos < root->match->length; ++pos)
	{
		while((goto_pos = _ac_goto(state, text[pos])) == FAIL) state = state->failure;
		state = state->next[goto_pos];

		if(state->rule != NONE || state->additional_size > 0)
		{
			_ac_add_match(&(root->match), state->rule);

			for(unsigned i = 0; i < state->additional_size; ++i)
			{
				_ac_add_match(&(root->match), state->additional_rule[i]);
			}

			root->match->position = pos + 1;
			root->match->state = state;

			return root->match;
		}
	}

	return NULL;
}

// TODO možná si budu držet rule interně a uživatel ho nebude zadávat
/*
 * @brief Add pattern to matching structure
 * @param root
 * @param text pattern
 * @param rule number of rule, this will be returned by match in results array
 */
 void add(pm_root * root, pm_keyword keywords[], unsigned count)
{
	size_t longest_match_length;
	size_t length;

	char * text;

	_ac_state * state;
	_ac_state * parent;
	_ac_state * new;

	for(unsigned j = 0; j < count; ++j)
	{
		text = keywords[j].text;
		length = keywords[j].length;

		state = _ac_longest_match(root, text, length, &longest_match_length);
		parent = state;
		new = NULL;


		for(unsigned i = 0; i < length - longest_match_length; ++i)
		{
			new = _ac_create();
			_ac_append(root, new, parent, text[longest_match_length + i]);
			parent = new;
		}

		new->rule = keywords[j].rule;
	}

	_ac_construct_failure(root, NONE);
}

void pm_remove(pm_root * root, char * text, unsigned length)
{
	_ac_state * state = root->state;
	_ac_state * saved_state = state;
	char saved_character = *text;
	_AC_RULE removed_rule = NONE;


	for(unsigned i = 0; i < length; ++i)
	{
		if(state->path_count > 0)
		{
			saved_state = state;
			saved_character = text[i];
		}

		state = state->next[_ac_goto(state, text[i])];
	}

	// keyword to be removed is prefix for longer keyword, delete just rule for this state
	if(state->key != NULL)
	{
		removed_rule = state->rule;
		state->rule = 0;
	}
	// this part of branch is only for this keyword and can be removed
	else
	{
		// remove path from previous state to this branch
		_ac_remove(root, saved_state, saved_character, &removed_rule);
	}

	_ac_construct_failure(root, removed_rule);
}

/*
 * @brief remove datastructures from memory
 * @param root
 */
void destroy(pm_root * root)
{
	for(unsigned i = 0; i < root->state->path_count; ++i)
	{
		if(root->state->next[i] == root->state) continue;
		_ac_destroy(root->state->next[i]);
	}

	_ac_free(root->state);

	free(root->match->rule);
	free(root->match);
	free(root->queue);
	free(root);
}
