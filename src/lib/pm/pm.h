#include <stdint.h>
#include <stdbool.h>
#include "types-precompiled.h"

pm_root * pm_init(pm_result ** result);
_Bool pm_match(pm_root * root, pm_result * result, char * text, unsigned length);
_Bool pm_match_next(pm_result * result);
void pm_add(pm_root * root, pm_keyword keywords[], unsigned size);
void pm_destroy(pm_root * root, pm_result * result);
void pm_remove(pm_root * root, char * text, unsigned length);
