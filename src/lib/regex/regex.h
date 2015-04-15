#include "types-precompiled.h"

regex_pattern * parse(char * input, unsigned length, unsigned char regex_number);
int match(regex_pattern * pattern, char * input, unsigned length);
regex_pattern * regex_construct(char * pattern);
_Bool regex_match(regex_pattern * pattern, char * text);
void regex_free(regex_pattern * pattern);
