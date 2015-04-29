#include "types-precompiled.h"
#include "../general/common.h"

regex_nfa * regex_construct_nfa(regex_pattern patterns[], unsigned count);
regex_dfa * regex_construct_dfa(regex_pattern patterns[], unsigned count);
int regex_match_nfa(regex_nfa * root, char * input, unsigned length);
int regex_match_dfa(regex_dfa * root, char * input, unsigned length);
void regex_destroy_nfa(regex_nfa * root);
void regex_destroy_dfa(regex_dfa * root);
