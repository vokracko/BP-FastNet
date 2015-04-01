#include "types-precompiled.h"

pm_root * pm_init();
pm_result * pm_match(pm_root * root, char * text, unsigned length);
pm_result * pm_match_next(pm_root * root);
void pm_add(pm_root * root, pm_keyword keywords[], unsigned size);
void pm_destroy(pm_root * root);
void pm_remove(pm_root * root, char * text, unsigned length);
