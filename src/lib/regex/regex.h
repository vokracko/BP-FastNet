#include "types-precompiled.h"

regex_pattern * regex_construct(char * pattern);
bool regex_match(regex_pattern * pattern, char * text);
void regex_free(regex_pattern * pattern);
