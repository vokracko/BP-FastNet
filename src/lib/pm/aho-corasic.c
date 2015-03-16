#include "aho-corasic.h"


// {

// 	for(unsigned i = 0; i < strlen(node->key); ++i)
// 	{
// 		node->next[i]->fallback = prev->fallback[]

// 		if(node->key != NULL)
// 		{
// 			// recurse
// 		}
// 	}
// }



/**
 * @brief Construct node
 * @return pointer to node
 */
_ac_node * _ac_create()
{
	_ac_node * node = (_ac_node *) malloc(sizeof(_ac_node));
	node->rule = 0;
	node->key = malloc(sizeof(char));
	node->key[0] = '\0';
	node->next = NULL;
	node->fallback = NULL;

	return node;
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
void _ac_append(_ac_node * node, _ac_node * parent, char character)
{
	// resize arrays
	size_t size = strlen(parent->key) + 1;
	parent->key = realloc(parent->key, size + 1); // for extra \0
	parent->next = realloc(parent->next, size * sizeof(_ac_node *));
	// and save new values
	parent->key[size - 1] = character;
	parent->key[size] = '\0';
	parent->next[size - 1] = node;
}

/*
 * @brief Go trought tree until there is a path to take
 * @param node start point
 * @param text source text for which path is searched
 * @param length[out] length of matched path
 * @return last matched node
 */
_ac_node * _ac_longest_match(_ac_node * node, char * text, size_t * length)
{
	char * pos;
	*length = 0;

	while(*text != '\0' && (pos = strchr(node->key, *text)) != NULL)
	{
		++text;
		++*length;
		node = node->next[pos - node->key];
	}

	return node;
}

/*
 * @brief Find fallback node with longest match
 * @param root
 * @param node processed node
 * @param text text for which is fallback path searched
 * @param endpos end position in text where fallback lookup should stop
 */
void _ac_fallback(pm_root * root, _ac_node * node, char * text,  size_t endpos)
{
	// default fallback is root node
	_ac_node * fallback	= root->node;
	size_t length;

	if(root->fallback_buffer_size < endpos + 1)
	{
		root->fallback_buffer_size <<= 1;
		root->fallback_buffer = realloc(root->fallback_buffer, root->fallback_buffer_size);
	}

	for(unsigned j = 0; j < endpos; ++j)
	{
		fallback = root->node;

		for(unsigned i = 0; i < endpos; ++i)
		{
			root->fallback_buffer[i] = text[endpos-i];
		}

		root->fallback_buffer[endpos] = '\0';
		fallback = _ac_longest_match(root->node, root->fallback_buffer, &length);

		if(fallback != root->node)
		{
			break;
		}
	}

	node->fallback = fallback;
}

void _ac_destroy(_ac_node * node)
{
	for(unsigned i = 0; i < strlen(node->key); ++i)
	{
		_ac_destroy(node->next[i]);
	}

	free(node->key);
	free(node->next);
	free(node);
}

/*
 * @brief Init data structures
 * @return root
 */
pm_root * init()
{
	pm_root * root;

	root = malloc(sizeof(pm_root));
	root->node = _ac_create();
	root->node->fallback = root->node;
	root->fallback_buffer = malloc(10);
	root->fallback_buffer_size = 10;
	root->matches = malloc(sizeof(_AC_RULE) * 10);
	root->matches_size = 10;

	return root;
}

int _ac_goto(ac_state * state, unsigned char character)
{
	char * pos = strchr(state->key, character);

	if(!pos) return NULL;

	return pos - state->key;
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
	int goto_pos;

	for(size_t pos = 0; pos < strlen(text); ++pos)
	{
		while((goto_pos = _ac_goto(state, text[pos])) == FAIL) state = _ac_failure(state);
		state = _ac_goto(state, text[pos]);

		// TODO printit výstup? zjistit od kořenka
		if(_ac_output(state) != NULL) print("%d - %s", pos, _ac_output(state));
	}

	return 0; //TODO co vracet?
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
	_ac_node * node = _ac_longest_match(root->node, text, &longest_match_length);
	_ac_node * parent = node;
	_ac_node * new = NULL;

	for(unsigned i = 0; i < length - longest_match_length; ++i)
	{
		new = _ac_create();
		_ac_fallback(root, new, text, longest_match_length + i);
		_ac_append(new, parent, text[longest_match_length + i]);
		parent = new;
	}

	/*
	new = _ac_create();
	_ac_fallback(new, text, strlen(text));
	_ac_append(new, parent, text[strlen(text) - 1]);
	*/
	new->rule = rule;
}

/*
 * @brief remove datastructures from memory
 * @param root
 */
void destroy(pm_root * root)
{
	_ac_destroy(root->node);
	free(root->fallback_buffer);
	free(root->matches);
	free(root);
}
