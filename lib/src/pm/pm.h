#include <stdint.h>
#include "types-precompiled.h"
#include "../common/common.h"

pm_root * pm_init();
_Bool pm_match(pm_root * root, char * input, unsigned length, pm_result * result);
_Bool pm_match_next(pm_result * result);
_Bool pm_add(pm_root * root, pm_keyword keywords[], unsigned size);
void pm_destroy(pm_root * root);
_Bool pm_remove(pm_root * root, char * text, unsigned length);
pm_result * pm_result_init();
void pm_result_destroy(pm_result * result);
