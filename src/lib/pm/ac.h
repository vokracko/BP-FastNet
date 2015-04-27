#include <stdint.h>
#include <stdio.h>
#include "../general/queue.h"
#include "pm.h"

#define FAIL -1

_Bool _ac_remove_rule(_ac_state *state, uint32_t rule);
int _ac_goto(_ac_state *state, char character);
_Bool _ac_append_rule(_ac_state *state, uint32_t rule);
_Bool _ac_construct_failure(pm_root *root, uint32_t removed_rule);
_ac_state *_ac_create(void);
_Bool _ac_add_match(pm_result *result, uint32_t matched_rule);
_Bool _ac_append(pm_root *root, _ac_state *state, _ac_state *parent, char character);
_ac_state *_ac_longest_match(pm_root *root, char *keyword_content, unsigned size, size_t *length);
void _ac_free(_ac_state *state);
_Bool _ac_remove(pm_root *root, _ac_state *prev, char character, uint32_t *removed_rule);
void _ac_destroy(_ac_state *state);
pm_root *pm_init(void);
pm_result *pm_result_init(void);
void pm_result_destroy(pm_result *result);
_Bool pm_match(pm_root *root, char *input, unsigned length, pm_result *result);
_Bool pm_match_next(pm_result *result);
_Bool pm_add(pm_root *root, pm_keyword keywords[], unsigned count);
_Bool pm_remove(pm_root *root, char *keyword_content, unsigned length);
void pm_destroy(pm_root *root);
