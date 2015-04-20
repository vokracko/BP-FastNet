#include <stdint.h>
#include "types-precompiled.h"
#include "../general/common.h"

pm_root * pm_init(pm_result ** result);
_Bool pm_match(pm_root * root, pm_result * result, char * text, unsigned length);
_Bool pm_match_next(pm_result * result);
_Bool pm_add(pm_root * root, pm_keyword keywords[], unsigned size);
void pm_destroy(pm_root * root, pm_result * result);
_Bool pm_remove(pm_root * root, char * text, unsigned length);
