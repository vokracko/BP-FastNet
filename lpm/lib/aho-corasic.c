#include "aho-corasic.h"

_ac_node * _ac_root;


_ac_node * _ac_create()
{
	_ac_node * node = (_ac_node *) malloc(sizeof(_ac_node));
	node->rule = 0;
	node->key = NULL;
	node->next = NULL;
	node->fall = NULL;

	return node;
}

void _ac_add_match(uint8_t ** matches, uint8_t * size, _AC_RULE matched_rule)
{
	*matches = (_AC_RULE *) realloc(*matches, *size + 1);
	(*matches)[(*size)++] = matched_rule;
}

_ac_node * _ac_longest_match(char * text, size_t * length)
{
	_ac_node * node = _ac_root;
	*length = 0;

	while(*text != '\0' && (pos = strstr(node->key, *text)) != NULL)
	{
		++text;
		++*length;
		node = node->next[pos - node->key];
	}

	return node;
}


void _ac_fallback(_ac_node * node, char * text,  size_t endpos)
{
	_ac_node * fallback	= _ac_root;
	size_t length;
	char * buffer = malloc(strlen(text) - endpos);
	
	for(int = 0; j < endpos; ++j)
	{
		fallback = _ac_root;
	}
	for(int i = j; i < endpos; ++i)
	{
		buffer[i] = text[endpos-i];
	}

	node = _ac_longest_match(buffer, &length);

	if(node != _ac_root)
	{
		node->fallback = fallbackl;
		return;
	}

}

void init()
{
	_ac_root = _ac_create();
	_ac_root->fall = _ac_root;
}

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
			node = node->fall;
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
void add(char * text, _AC_RULE rule)
{
	size_t length;
	size_t index = strlen(text);
	_ac_node * node = _ac_longest_match(text, &length);

	if(length == strlen(text))
	{
		node->rule = rule;
		return;
	}

	// todo vytvořit část konenčného automatu, který bude odpovídat tomuto textu
	// doplnit fail cesty
	// pravděpodobně procházet od konce stringu a postupně vytvářet

	_ac_fallback(node, text, length);
	// TODO otestovat
	_ac_add_node();
}
