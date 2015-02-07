#include "tbm.h"

_tbm_node * _tbm_root;

void lpm_init(_LPM_RULE_SIZE default_rule, _LPM_RULE_SIZE default_rule6)
{
	_tbm_root = malloc(sizeof(_tbm_node));
	// _tbm_root->child = malloc(sizeof(_tbm_node) * _TBM_SIZE_EXTERNAL);
	_tbm_root->child = NULL;
	_tbm_root->rule = malloc(sizeof(_tbm_rule) * _TBM_SIZE_INTERNAL);
	//TODO kontrola na null
}

void lpm_add(uint32_t prefix, uint8_t prefix_len, _LPM_RULE_SIZE rule);
void lpm_update(uint32_t prefix, uint8_t prefix_len, _LPM_RULE_SIZE rule);
void lpm_remove(uint32_t prefix, uint8_t prefix_len);
void lpm_destroy();

uint32_t lpm_lookup(uint32_t key)
{
	_tbm_node * node = _tbm_root;
	uint8_t position = 0;
	uint8_t bits;
	uint8_t index;
	_tbm_node * best_match_node = _tbm_root;
	uint8_t best_match_index = 0;
	uint8_t length;


	while(true)
	{
		length = STRIDE;
		bits = get_bits(key, position++);

		// no child, return best matched prefix so far
		if(!node->external[bits]) return best_match_node->rule[best_match_index];

		// index for child pointer is bitsum from <0, (int) bits)
		index = _tbm_bitsum(node->external, 0, bits);

		// from longest match to shortest: NNN, NN*, N*, *
		// node->internal => no "ones"
		while(length >= 0 && node->internal[bits] == 0)
		{
			bits = bits >> 1;
		}

		// index for prefix pointer is bitsum from <0, index(length, (int) bits))
		best_match_index = _tbm_bitsum(node->internal, length, bits);
		best_match_node = node;
		// move to child
		node = node->child[index];
	}
}

// TODO optimalizace http://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer
inline uint8_t _tbm_bitsum(uint32_t key, uint8_t bit_length, bit_value)
{
	// cut off the bits at endpos and after
	uint8_t source = key >> (1 << bit_length - 1 + bit_value);
	return __builtin_popcount(source);
}

//TODO nějak řešit konec pokud není 32 přímo dělitelné střídou?
