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

int _ac_goto(_ac_state * state, unsigned char character)
{
	char * pos = strchr(state->key, character);

	if(!pos) return FAIL;

	return pos - state->key;
}

bool _ac_queue_empty(pm_root * root)
{
	return root->queue->head == NULL;
}

void _ac_queue_insert(pm_root * root, _ac_state * state)
{
	// TODO nějaká fronta s obsazenými a volnými?? ať se pořád nealokuje
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
	for(unsigned i = 0; i < strlen(state->key); ++i)
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

		unsigned length = strlen(r->key);

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
	state->key = malloc(sizeof(char));
	state->key[0] = '\0';
	state->next = NULL;
	state->failure = NULL;

	return state;
}

/**
 * @brief insert matched rule to matches
 * @param root
 * @param size size of matches
 * @param matched_rule
 */
void _ac_add_match(pm_root * root, unsigned * size, _AC_RULE matched_rule)
{
	if(root->matches_size < *size + 1)
	{
		root->matches_size <<= 1;
		root->matches = (_AC_RULE *) realloc(root->matches, root->matches_size * sizeof(_AC_RULE));
	}

	root->matches[(*size)++] = matched_rule;
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
		parent->next[character - 'a'] = state;
		return;
	}
	// resize arrays
	size_t size = strlen(parent->key) + 1;
	parent->key = realloc(parent->key, size + 1); // for extra \0
	parent->next = realloc(parent->next, size * sizeof(_ac_state *));
	// and save new values
	parent->key[size - 1] = character;
	parent->key[size] = '\0';
	parent->next[size - 1] = state;
}

/*
 * @brief Go trought tree until there is a path to take
 * @param node start point
 * @param text source text for which path is searched
 * @param length[out] length of matched path
 * @return last matched node
 */
_ac_state * _ac_longest_match(pm_root * root, char * text, size_t * length)
{
	int goto_pos;
	_ac_state * state = root->state;
	*length = 0;

	while(*text != '\0' && (goto_pos = _ac_goto(state, *text)) != FAIL)
	{
		state = state->next[goto_pos];

		if(state == root->state) break;
		++text;
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

// TODO pokud bude root tak pouze nastavit na root, neposouvat a nic nemazat
/**
 * @brief Remove branch from prev
 */
void _ac_remove(_ac_state * prev, char character, _AC_RULE * removed_rule)
{
	_ac_state * state;
	char * pos;
	size_t key_length;

	pos = strchr(prev->key, character);
	key_length = strlen(prev->key);

	for(unsigned i = pos - prev->key; i < key_length; ++i)
	{
		prev->key[i] = prev->key[i + 1];
		prev->next[i] = prev->next[i + 1];
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
	for(unsigned i = 0; i < strlen(state->key); ++i)
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
	root->state->key = realloc(root->state->key, 'z' - 'a' + 2); // + \0
	root->state->next = malloc(('z' - 'a' + 1) * sizeof(_ac_state *));

	for(unsigned i = 'a'; i <= 'z'; ++i)
	{
		root->state->key[i - 'a'] = i;
		root->state->next[i - 'a'] = root->state;
	}

	root->state->key['z' - 'a' + 1] = '\0';

	root->matches = malloc(sizeof(_AC_RULE) * 10);
	root->matches_size = 10;

	root->queue = malloc(sizeof(_ac_queue));
	root->queue->head = NULL;
	root->queue->tail = NULL;

	return root;
}

/*
 * @brief Go trought text and save matched patterns
 * @param root
 * @param text input string
 * @param matches[out] array of matches
 * @return array of matches
 */
unsigned match(pm_root * root, char * text, _AC_RULE ** matches)
{
	_ac_state * state = root->state;
	unsigned size = 0;
	int goto_pos;

	for(size_t pos = 0; pos < strlen(text); ++pos)
	{
		while((goto_pos = _ac_goto(state, text[pos])) == FAIL) state = state->failure;
		state = state->next[goto_pos];

		// TODO printit výstup? zjistit od kořenka
		_ac_add_match(root, &size, state->rule);

		for(unsigned i = 0; i < state->additional_size; ++i)
		{
			_ac_add_match(root, &size, state->additional_rule[i]);
		}
	}

	*matches = root->matches;

	return size;
}

// TODO možná si budu držet rule interně a uživatel ho nebude zadávat
/*
 * @brief Add pattern to matching structure
 * @param root
 * @param text pattern
 * @param rule number of rule, this will be returned by match in results array
 */
void add(pm_root * root, char * text, _AC_RULE rule)
{
	size_t longest_match_length;
	size_t length = strlen(text);
	_ac_state * state = _ac_longest_match(root, text, &longest_match_length);
	_ac_state * parent = state;
	_ac_state * new = NULL;

	for(unsigned i = 0; i < length - longest_match_length; ++i)
	{
		new = _ac_create();
		_ac_append(root, new, parent, text[longest_match_length + i]);
		parent = new;
	}

	new->rule = rule;

	_ac_construct_failure(root, NONE);
}

void pm_remove(pm_root * root, char * text)
{
	_ac_state * state = root->state;
	_ac_state * prev;
	_AC_RULE removed_rule = NONE;

	while(*text != '\0' && strlen(state->key) > 1)
	{
		prev = state;
		state = state->next[_ac_goto(state, *text++)];
	}

	// keyword to be removed is prefix for longer keyword, delete just rule for this state
	if(*text == '\0' && state->key != NULL)
	{
		removed_rule = state->rule;
		state->rule = 0;
	}
	// this part of branch is only for this keyword and can be removed
	else
	{
		// remove path from previous state to this branch
		_ac_remove(prev, *(text - 1), &removed_rule);
	}

	// TODO remove všechny zmínky o mazaném state->rule z additional
	_ac_construct_failure(root, removed_rule);
}

/*
 * @brief remove datastructures from memory
 * @param root
 */
void destroy(pm_root * root)
{
	for(unsigned i = 0; i < strlen(root->state->key); ++i)
	{
		if(root->state->next[i] == root->state) continue;
		_ac_destroy(root->state->next[i]);
	}

	_ac_free(root->state);

	free(root->matches);
	free(root->queue);
	free(root);
}
