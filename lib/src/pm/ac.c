#include "ac.h"

_Bool _ac_remove_rule(_ac_state * state, PM_RULE rule)
{
	PM_RULE * new_additional_rule = NULL;

	assert(state != NULL);

	for(unsigned i = 0; i < state->additional_size; ++i)
	{
		if(state->additional_rule[i] == rule)
		{
			for(unsigned j = i; j < state->additional_size - 1; ++j)
			{
				state->additional_rule[j] = state->additional_rule[j + 1];
			}

			new_additional_rule = realloc(state->additional_rule, sizeof(PM_RULE) * (state->additional_size - 1));

			// if new additional_size == 0 then it should return NULL
			if(new_additional_rule == NULL && state->additional_size != 1) return errno = FASTNET_OUT_OF_MEMORY, 0;

			state->additional_rule = new_additional_rule;
			--state->additional_size;

			return 1;
		}
	}

	return 1;
}

int _ac_goto(_ac_state * state, char character)
{
	assert(state != NULL);

	char * pos = memchr(state->key, character, state->path_count);

	if(pos == NULL) return FAIL;

	return pos - state->key;
}

_Bool _ac_append_rule(_ac_state * state, PM_RULE rule)
{
	PM_RULE * new_additional_rule;

	assert(state != NULL);

	if(rule == PM_RULE_NONE) return 1;

	for(unsigned i = 0; i < state->additional_size; ++i)
	{
		if(state->additional_rule[i] == rule) return 1;
	}

	new_additional_rule = realloc(state->additional_rule, (state->additional_size + 1) * sizeof(PM_RULE));

	if(new_additional_rule == NULL) return errno = FASTNET_OUT_OF_MEMORY, 0;

	state->additional_rule = new_additional_rule;
	state->additional_rule[state->additional_size] = rule;
	++state->additional_size;

	return 1;
}


_Bool _ac_construct_failure(pm_root * root, PM_RULE removed_rule)
{
	assert(root != NULL);

	queue * queue_ = queue_init();

	if(queue_ == NULL) return 0;

	_ac_state * state = root;
	_ac_state * s, * r;
	int goto_pos;

	root->failure = root;

	// start constructing, go throught every direct follower of root state = depth 1
	for(unsigned i = 0; i < state->path_count; ++i)
	{
		if(state->next[i] == root) continue;

		// depth 1, failure for every state at this depth is root state
		state->next[i]->failure = root;

		// "every path in root state is defined" => skip those with no real follower
		if(queue_insert(queue_, state->next[i], POINTER) == 0) return queue_destroy(queue_), 0;
	}


	while(!queue_empty(queue_))
	{
		r = queue_front(queue_).pointer;

		unsigned length = r->path_count;

		// for every follower
		for(unsigned i = 0; i < length; ++i)
		{
			s = r->next[i];

			if(queue_insert(queue_, s, POINTER) == 0) return queue_destroy(queue_), 0;
			state = r->failure;
			// find failure path
			while((goto_pos = _ac_goto(state, r->key[i])) == FAIL)
			{
				state = state->failure;
			}

			s->failure = state->next[goto_pos];

			if(_ac_remove_rule(s, removed_rule) == 0) return queue_destroy(queue_), 0;
			if(_ac_append_rule(s, s->failure->rule) == 0) return queue_destroy(queue_), 0;
			// copy all rules from failure state to this state
			for(unsigned j = 0; j < s->failure->additional_size; ++j)
			{
				if(_ac_append_rule(s, s->failure->additional_rule[j]) == 0) return queue_destroy(queue_), 0;
			}
		}
	}

	queue_destroy(queue_);

	return 1;
}



/**
 * @brief Construct node
 * @return pointer to node
 */
_ac_state * _ac_create()
{
	_ac_state * state = (_ac_state *) malloc(sizeof(_ac_state));

	if(state == NULL) return errno = FASTNET_OUT_OF_MEMORY, NULL;

	state->rule = PM_RULE_NONE;
	state->additional_rule = NULL;
	state->additional_size = 0;
	state->key = NULL;
	state->path_count = 0;
	state->next = NULL;
	state->failure = NULL;

	return state;
}


_Bool _ac_add_match(pm_result * result, PM_RULE matched_rule)
{
	assert(result != NULL);

	PM_RULE * new_rule;

	if(result->size == result->count)
	{
		new_rule = realloc(result->rule, (result->size << 1) * sizeof(PM_RULE));

		if(new_rule == NULL) return errno = FASTNET_OUT_OF_MEMORY, 0;

		result->size <<= 1;
		result->rule = new_rule;
	}

	result->rule[result->count++] = matched_rule;

	return 1;
}

/*
 * @brief Append node to parent and resize array of next pointers & keys
 * @param node
 * @param parent
 * @param character
 */
_Bool _ac_append(pm_root * root, _ac_state * state, _ac_state * parent, char character)
{
	assert(root != NULL);
	assert(state != NULL);
	assert(parent != NULL);

	char * new_key;
	_ac_state ** new_next;

	if(parent == root)
	{
		parent->next[ (unsigned char) character] = state;
		return 1;
	}
	// resize arrays
	new_key = realloc(parent->key, parent->path_count + 1);

	if(new_key == NULL) return errno = FASTNET_OUT_OF_MEMORY, 0;

	new_next = realloc(parent->next, (parent->path_count + 1) * sizeof(_ac_state *));

	if(new_next == NULL) return errno = FASTNET_OUT_OF_MEMORY, free(new_key), 0;

	parent->key = new_key;
	parent->next = new_next;
	parent->path_count++;
	// and save new values
	parent->key[parent->path_count - 1] = character;
	parent->next[parent->path_count - 1] = state;

	return 1;
}

/*
 * @brief Go trought tree until there is a path to take
 * @param node start point
 * @param text source text for which path is searched
 * @param length[out] length of matched path
 * @return last matched node
 */
_ac_state * _ac_longest_match(pm_root * root, char * keyword_content, unsigned size, size_t * length)
{
	assert(root != NULL);
	assert(length != NULL);

	int goto_pos;
	_ac_state * state = root;
	*length = 0;
	unsigned i = 0;

	while(i < size && (goto_pos = _ac_goto(state, keyword_content[i])) != FAIL)
	{
		state = state->next[goto_pos];

		if(state == root) break;
		++i;
		++*length;

	}

	return state;
}

void _ac_free(_ac_state * state)
{
	assert(state != NULL);

	free(state->key);
	free(state->next);
	free(state->additional_rule);
	free(state);
}

/**
 * @brief Remove branch from prev
 */
_Bool _ac_remove(pm_root * root, _ac_state * prev, char character, PM_RULE * removed_rule)
{
	assert(root != NULL);
	assert(prev != NULL);

	_ac_state * state;
	char * pos;
	size_t key_length;
	_ac_state ** new_next;
	char * new_key;

	key_length = prev->path_count;
	--prev->path_count;
	pos = memchr(prev->key, character, key_length);
	state = prev->next[pos - prev->key];

	if(prev == root)
	{
		prev->next[pos - prev->key] = root;
	}
	else
	{
		for(unsigned i = pos - prev->key; i < key_length - 1; ++i)
		{
			prev->key[i] = prev->key[i + 1];
			prev->next[i] = prev->next[i + 1];
		}

		new_key = realloc(prev->key, prev->path_count);
		// if prev->path_cout == 0 then it should return NULL
		if(new_key == NULL && prev->path_count != 0) return errno = FASTNET_OUT_OF_MEMORY, 0;
		prev->key = new_key;

		new_next = realloc(prev->next, prev->path_count * sizeof(_ac_state *));
		// if prev->path_cout == 0 then it should return NULL
		if(new_next == NULL && prev->path_count != 0) return errno = FASTNET_OUT_OF_MEMORY, 0;
		prev->next = new_next;
	}


	while(state->next != NULL)
	{
		prev = state;
		state = state->next[0];
		_ac_free(prev);
	}

	*removed_rule = state->rule;
	_ac_free(state);

	return 1;
}

void _ac_destroy(_ac_state * state)
{
	assert(state != NULL);

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
pm_root * pm_init()
{
	pm_root * root = _ac_create();

	if(root == NULL) return errno = FASTNET_OUT_OF_MEMORY, NULL;

	root->key = malloc(256);

	if(root->key == NULL) return errno = FASTNET_OUT_OF_MEMORY, free(root), NULL;

	root->next = malloc(256 * sizeof(_ac_state *));

	if(root->next == NULL) return errno = FASTNET_OUT_OF_MEMORY, free(root->key), free(root), NULL;

	root->path_count = 256;

	for(unsigned i = 0; i < 256; ++i)
	{
		root->key[i] = i;
		root->next[i] = root;
	}

	return root;
}

pm_result * pm_result_init()
{
	pm_result * result = malloc(sizeof(pm_result));

	if(result == NULL) return errno = FASTNET_OUT_OF_MEMORY, NULL;

	result->rule = malloc(sizeof(PM_RULE) * 10);

	if(result->rule == NULL) return errno = FASTNET_OUT_OF_MEMORY, free(result), NULL;
	result->count = 0;
	result->size = 10;

	return result;
}

void pm_result_destroy(pm_result * result)
{
	free(result->rule);
	free(result);
}

_Bool pm_match(pm_root * root, char * input, unsigned length, pm_result * result)
{
	assert(root != NULL);
	assert(input != NULL);
	assert(result != NULL);

	_ac_state * state = root;
	int goto_pos;

	for(size_t pos = 0; pos < length; ++pos)
	{
		while((goto_pos = _ac_goto(state, input[pos])) == FAIL) state = state->failure;
		state = state->next[goto_pos];

		if(state->rule != PM_RULE_NONE || state->additional_size > 0)
		{
			if(result == NULL) return 1;

			if(_ac_add_match(result, state->rule) == 0) return errno = FASTNET_OUT_OF_MEMORY, 1;

			for(unsigned i = 0; i < state->additional_size; ++i)
			{
				if(_ac_add_match(result, state->additional_rule[i]) == 0) return errno = FASTNET_OUT_OF_MEMORY, 1;
			}

			result->state = state;
			result->position = pos + 1;

			result->input = input;
			result->length = length;

			return 1;
		}
	}

	return 0;
}

_Bool pm_match_next(pm_result * result)
{
	assert(result != NULL);

	result->count = 0;

	return pm_match(result->state, result->input + result->position, result->length - result->position, result);
}

/*
 * @brief Add pattern to matching structure
 * @param root
 * @param text pattern
 * @param rule number of rule, this will be returned by match in results array
 */
_Bool pm_add(pm_root * root, pm_keyword keywords[], unsigned count)
{
	assert(root != NULL);
	assert(keywords != NULL);

	size_t longest_match_length;
	size_t length;

	char * keyword_content;

	_ac_state * state;
	_ac_state * parent;
	_ac_state * new;

	for(unsigned j = 0; j < count; ++j)
	{
		keyword_content = keywords[j].content;
		length = keywords[j].length;

		state = _ac_longest_match(root, keyword_content, length, &longest_match_length);
		parent = state;
		new = NULL;

		for(unsigned i = 0; i < length - longest_match_length; ++i)
		{
			new = _ac_create();

			if(new == NULL) return pm_destroy(root), 0;
			if(_ac_append(root, new, parent, keyword_content[longest_match_length + i]) == 0)
				return pm_destroy(root), _ac_free(new), 0;
			parent = new;
		}

		if(new != NULL) new->rule = keywords[j].rule;
	}

	return _ac_construct_failure(root, PM_RULE_NONE);
}

_Bool pm_remove(pm_root * root, char * keyword_content, unsigned length)
{
	assert(root != NULL);
	assert(keyword_content != NULL);

	_ac_state * state = root;
	_ac_state * saved_state = state;
	char saved_character = *keyword_content;
	PM_RULE removed_rule = PM_RULE_NONE;


	for(unsigned i = 0; i < length; ++i)
	{
		if(state->path_count > 0)
		{
			saved_state = state;
			saved_character = keyword_content[i];
		}
		state = state->next[_ac_goto(state, keyword_content[i])];
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
		if(_ac_remove(root, saved_state, saved_character, &removed_rule) == 0) return 0;
	}

	return _ac_construct_failure(root, removed_rule);
}

/*
 * @brief remove datastructures from memory
 * @param root
 */
void pm_destroy(pm_root * root)
{
	assert(root != NULL);

	for(unsigned i = 0; i < root->path_count; ++i)
	{
		if(root->next[i] == root) continue;
		_ac_destroy(root->next[i]);
	}

	_ac_free(root);
}
