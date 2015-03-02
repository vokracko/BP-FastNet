#include "aho-corasic.h"

_ac_node * _ac_root;


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
 * @param matches array of matches
 * @param size size of matches
 * @param matched_rule
 */
void _ac_add_match(uint8_t ** matches, uint8_t * size, _AC_RULE matched_rule)
{
	*matches = (_AC_RULE *) realloc(*matches, *size + 1);
	(*matches)[(*size)++] = matched_rule;
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
 * @param text source text for which path is searched
 * @param length[out] length of matched path
 * @return last matched node
 */
_ac_node * _ac_longest_match(char * text, size_t * length)
{
	_ac_node * node = _ac_root;
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
 * @param node processed node
 * @param text text for which is fallback path searched
 * @param endpos end position in text where fallback lookup should stop
 */
void _ac_fallback(_ac_node * node, char * text,  size_t endpos)
{
	// default fallback is root node
	_ac_node * fallback	= _ac_root;
	size_t length;
	char * buffer = malloc(endpos+1);

	for(int j = 0; j < endpos; ++j)
	{
		fallback = _ac_root;

		for(int i = 0; i < endpos; ++i)
		{
			printf("text: %s. endpos: %d, i: %d\n", text, endpos, i);
			buffer[i] = text[endpos-i];
		}

		buffer[endpos] = '\0';
		node = _ac_longest_match(buffer, &length);

		if(node != _ac_root)
		{
			break;
		}
	}

	free(buffer);
	node->fallback = fallback;
}

void _ac_destroy(_ac_node * node)
{
	for(int i = 0; i < strlen(node->key); ++i)
	{
		_ac_destroy(node->next[i]);
	}

	free(node->key);
	free(node->next);
	free(node);
}

/*
 * @brief Init data structures
 */
void init()
{
	_ac_root = _ac_create();
	_ac_root->fallback = _ac_root;
}

/*
 * @brief Go trought text and save matched patterns
 * @param text input string
 * @param matches[out] array of matches
 * @return array of matches
 * @todo udržovat matches a nerealokovat pořád
 */
uint8_t match(char * text, _AC_RULE ** matches)
{
	char * pos;
	uint8_t size = 0;
	_ac_node * node = _ac_root;

	while(*text != '\0')
	{
		pos = strchr(node->key, *text);

		// rule matched, add it to result set
		if(node->rule != 0)
		{
			_ac_add_match(matches, &size, node->rule);
		}

		// no way to continue current way, move to the fall node
		if(pos == NULL)
		{
			node = node->fallback;
		}
		// continue in current way
		else
		{
			node = node->next[pos - node->key];
		}

		++text;
	}

	return size;
}

// TODO možná si budu držet rule interně a uživatel ho nebude zadávat
/*
 * @brief Add pattern to matching structure
 * @param text pattern
 * @param rule number of rule, this will be returned by match in results array
 */
void add(char * text, _AC_RULE rule)
{
	size_t longest_match_length;
	size_t length = strlen(text);
	_ac_node * node = _ac_longest_match(text, &longest_match_length);
	_ac_node * parent = node;
	_ac_node * new = NULL;
	// todo vytvořit část konenčného automatu, který bude odpovídat tomuto textu
	// doplnit fail cesty

	for(int i = 0; i < length - longest_match_length; ++i)
	{
		// TODO najít nejdelší shodu
		new = _ac_create();
		_ac_fallback(new, text, longest_match_length + i);
		_ac_append(parent, new, text[longest_match_length + i]);
		parent = new;
	}

	node->rule = rule;
}

/*
 * @brief remove datastructures from memory
 */
void destroy()
{
	_ac_destroy(_ac_root);
}
