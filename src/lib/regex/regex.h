#include "types-precompiled.h"

regex_root * regex_construct(regex_pattern patterns[], unsigned count);
int regex_match(regex_root * root, char * input, unsigned length);
void regex_destroy(regex_root * root);
