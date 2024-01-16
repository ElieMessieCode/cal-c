#ifndef CALC_VARS_H
#define CALC_VARS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "calc/value.h"
#include "calc/error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CALC_VARS_MAX_ENTRIES 256
#define CALC_VAR_NAME_MAX_LEN 32

typedef struct CalcVarEntry {
    char      name[CALC_VAR_NAME_MAX_LEN];
    CalcValue value;
    bool      is_const;
} CalcVarEntry;

typedef struct CalcVarTable {
    CalcVarEntry entries[CALC_VARS_MAX_ENTRIES];
    size_t       count;
} CalcVarTable;

void calc_vars_init(CalcVarTable *table);
bool calc_vars_get(const CalcVarTable *table, const char *name, CalcValue *out_val);
bool calc_vars_set(CalcVarTable *table, const char *name, const CalcValue *val, CalcError *err, CalcSpan span);
void calc_vars_set_ans(CalcVarTable *table, const CalcValue *val);
bool calc_vars_is_reserved(const char *name);
size_t calc_vars_count(const CalcVarTable *table);
bool calc_vars_get_entry(const CalcVarTable *table, size_t index, const CalcVarEntry **out_entry);

#ifdef __cplusplus
}
#endif

#endif
