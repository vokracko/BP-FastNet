#include "bspl.h"

uint128_t _bspl_default_rule6;
uint32_t _bspl_default_rule;

_bspl_node * _bspl_root;
_bspl_node6 * _bspl_root6;

_bspl_node ** _bspl_htable;
_bspl_node6 ** _bspl_htable6;

uint32_t calculate_hash(uint32_t key)
{
	uint32_t hash, i;

	for(hash = i = 0; i < 4; ++i)
	{
		hash += get_byte(key, i);
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash % _BSPL_HTABLE_SIZE;
}

void _bspl_leaf_pushing(_bspl_node * current, uint32_t rule_original, uint32_t rule_new)
{
	if(current->rule == rule_original)
	{
		current->rule = rule_new;
	}

	if(current->type == _BSPL_NODE_INTERNAL)
	{
		_bspl_leaf_pushing(current->left, rule_original, rule_new);
		_bspl_leaf_pushing(current->right, rule_original, rule_new);
	}
}

void _bspl_node_htable_add(_bspl_node * this)
{
	uint32_t index = calculate_hash(this->prefix);
	_bspl_node * ptr = _bspl_htable[index];

	if(ptr == NULL)
	{
		_bspl_htable[index] = this;
	}
	else
	{
		while(ptr->next != NULL) ptr = ptr->next;
		ptr->next = this;
	}
}

_bspl_node* _bspl_node_create()
{
	_bspl_node * node;
	// TODO malloc na null
	node = (_bspl_node *) malloc(sizeof(_bspl_node));
	node->left = NULL;
	node->right = NULL;
	node->next = NULL;
	node->type = _BSPL_NODE_CREATED;

	// TODO přidat do hash tabulky

	return node;
}

void _bspl_node_add(_bspl_node * parent, _bspl_node * this, bool bit)
{
	_bspl_node_htable_add(this);

	if(bit)
	{
		parent->right = this;
	}
	else
	{
		parent->left = this;
	}
}

// hledá ve stromové struktuře
_bspl_node * _bspl_lookup(uint32_t prefix, uint8_t prefix_len)
{
	uint32_t bit_position = 31;
	uint32_t len = 0;
	bool bit;
	_bspl_node * node = _bspl_root;

	do
	{
		bit = get_bit(prefix, bit_position--);

		if(bit)
		{
			node = node->right;
		}
		else
		{
			node = node->left;
		}

		++len;

	} while(len != prefix_len && node != NULL);

	return node;
}

void lpm_init(uint32_t default_rule, uint128_t default_rule6)
{
	_bspl_default_rule = default_rule;
	_bspl_default_rule6 = default_rule6;

	//TODO kontrola na null

	// INIT for IPv4
	_bspl_root = (_bspl_node *) malloc(sizeof(_bspl_node));
	_bspl_root->type = _BSPL_NODE_PREFIX;
	_bspl_root->rule = _bspl_default_rule;
	_bspl_root->left = NULL;
	_bspl_root->right = NULL;
	_bspl_root->next = NULL;
	_bspl_root->prefix = 0;

	_bspl_htable = (_bspl_node **) calloc(_BSPL_HTABLE_SIZE, sizeof(_bspl_node *));
	_bspl_node_htable_add(_bspl_root);
	_bspl_root->left = NULL;
	_bspl_root->right = NULL;
	_bspl_root->next = NULL;
	_bspl_root->prefix = 0;
	// INIT for IPv6
	// _bspl_root6 = (_bspl_node6 *) malloc(sizeof(_bspl_node6));
	// _bspl_root6->type = _BSPL_NODE_PREFIX;
	// _bspl_root6->rule = _bspl_default_rule6;
	// _bspl_root6->left = NULL;
	// _bspl_root6->right = NULL;
	// _bspl_root6->next = NULL;
	// _bspl_root6->prefix = 0;

	// _bspl_htable6 = (_bspl_node6 **) calloc(_BSPL_HTABLE_SIZE, sizeof(_bspl_node6 *));
	// _bspl_node6_htable_add(_bspl_root6); //TODO
}


/**
 * @brief Free all data structures allocated for BSPL
 */
void lpm_destroy()
{
	_bspl_node * current, * prev;
	_bspl_node6 * current6, * prev6;
	// IPv4
	for(int i = 0; i < _BSPL_HTABLE_SIZE; ++i)
	{
		current = _bspl_htable[i];

		while(current != NULL)
		{
			prev = current;
			current = current->next;

			free(prev);
		}

		// current6 = _bspl_htable6[i];

		// while(current6 != NULL)
		// {
		// 	prev6 = current6;
		// 	current6 = current6->next;

		// 	free(prev6);
		// }

	}

	free(_bspl_htable);
	// free(_bspl_htable6); //TODO
}

// TODO bude vracet na základě mallocu?

// vložení nového prefixu do struktury
//TODO bude vracet něco? pokud existuje/nepodařilo se vložit
void lpm_add(uint32_t prefix, uint8_t prefix_len, uint32_t rule)
{
	_bspl_node* current = NULL;
	_bspl_node* parent;
	_bspl_node* other;
	bool bit;
	uint8_t len = 0;
	uint32_t parent_rule;
	uint32_t prefix_bits;

	// if(current != NULL)
	// {
		// TODO řešit?
		// prvek s takovým prefixem již existuje, pouze ho updatuju
		// _lpm_update(current, rule);
		// return;
	// }

	parent = _bspl_root;

	do
	{
		bit = get_bit(prefix, 31 - len);
		prefix_bits = get_bits(parent->prefix, len);
		parent_rule = parent->rule;
		current =  bit ? parent->right : parent->left;
		other  =  bit ? parent->left : parent->right;
		parent->type = _BSPL_NODE_INTERNAL;

		if(other == NULL)
		{
			other = _bspl_node_create();
			other->type = _BSPL_NODE_PREFIX;
			other->prefix = prefix_bits | (!bit << (31 - len));
			other->prefix_len = len + 1;
			other->rule = parent_rule;
			_bspl_node_add(parent, other, !bit);

		}

		if(current == NULL)
		{
			current = _bspl_node_create();
			current->prefix = prefix_bits | (bit << (31 - len));
			current->prefix_len = len + 1;
			current->rule = parent_rule;
			_bspl_node_add(parent, current, bit);

			if(prefix_len == len + 1)
			{
				break;
			}

		}

		if(parent->type != _BSPL_NODE_INTERNAL) parent->type = _BSPL_NODE_INTERNAL;

		parent = current;
		++len;
	}
	while(prefix_len != len);

	if(current->type == _BSPL_NODE_CREATED)
	{
		current->type = _BSPL_NODE_PREFIX;
		current->rule = rule;
		current->prefix = get_bits(prefix, prefix_len);
		current->prefix_len = prefix_len;
	}
	else
	{
		_bspl_leaf_pushing(current, current->rule, rule);
	}
}


//TODO bude vracet něco? pokud neexistuje/nepodařilo se updatovat
void lpm_update(uint32_t prefix, uint8_t prefix_len, uint32_t rule)
{
	_bspl_node * node = _bspl_lookup(prefix, prefix_len);

	if(node == NULL)
	{
		lpm_add(prefix, prefix_len, rule);
	}
	else if(node->type == _BSPL_NODE_INTERNAL)
	{
		_bspl_leaf_pushing(node, node->rule, rule);
	}

	node->rule = rule;
}

void lpm_delete(uint32_t prefix, uint8_t prefix_len)
{
	// TODO
	// TODO
	_bspl_node * node = _bspl_lookup(prefix, prefix_len);
	//TODO smazat a leaf pushing
	//TODO mazat i v tabulce

	if(node->type == _BSPL_NODE_INTERNAL)
	{
		// TODO znamená že má oba potomky
		// TODO mazat i potomky, kteří zdědily routu
	}
	else
	{
		// TODO najít nadřazený kde bude mít prefix a ten sem pushnout
	}
}

void * _bspl_lookup_thread(void * key_ptr)
{
	uint32_t key = * (uint32_t *) key_ptr;
	uint32_t prefix_bits; // extracted part of prefix
	uint8_t prefix_len = 32; // binary search actual length
	uint8_t prefix_change = 32; // binary search length change

	_bspl_node * node = NULL;

	do
	{
		prefix_bits = get_bits(key, prefix_len);
		node = _bspl_htable[calculate_hash(prefix_bits)];

		while(node != NULL && (node->prefix != prefix_bits || node->prefix_len != prefix_len))
		{
			node = node->next;
		}

		prefix_change >>= 1;

		if(node == NULL) prefix_len -= prefix_change;
		else if(node->type == _BSPL_NODE_INTERNAL) prefix_len += prefix_change;
		else break;

	} while(prefix_change > 0);

	* (uint32_t *) key_ptr = node->rule;

	pthread_exit(NULL);
}

// // hledá v tabulce algoritmem BSPL
// TODO vrace thread_id + další fce get lookup result?
uint32_t lpm_lookup(uint32_t key)
{
	pthread_t thread_id;

	// TODO -pthread kompilace
	pthread_create(&thread_id, NULL, _bspl_lookup_thread, (void *) &key);
	pthread_join(thread_id, NULL);

	return key;

}
