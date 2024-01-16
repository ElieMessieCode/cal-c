#include "calc/vars.h"
#include <string.h>

static const char *const RESERVED_NAMES[] = {
    "pi", "e",
    "sin", "cos", "tan", "asin", "acos", "atan",
    "sqrt", "cbrt", "exp", "ln", "log", "log10",
    "abs", "floor", "ceil", "round",
    "min", "max", "pow", "hypot",
    NULL
};

bool calc_vars_is_reserved(const char *name) {
    if (name == NULL) {
        return false;
    }
    for (size_t i = 0; RESERVED_NAMES[i] != NULL; ++i) {
        if (strcmp(name, RESERVED_NAMES[i]) == 0) {
            return true;
        }
    }
    return false;
}

void calc_vars_init(CalcVarTable *table) {
    if (table == NULL) {
        return;
    }
    memset(table, 0, sizeof(*table));

    // pi
    strncpy(table->entries[0].name, "pi", CALC_VAR_NAME_MAX_LEN - 1);
    table->entries[0].value = calc_value_from_float(3.141592653589793238462643383279502884);
    table->entries[0].is_const = true;

    // e
    strncpy(table->entries[1].name, "e", CALC_VAR_NAME_MAX_LEN - 1);
    table->entries[1].value = calc_value_from_float(2.718281828459045235360287471352662497);
    table->entries[1].is_const = true;

    // ans
    strncpy(table->entries[2].name, "ans", CALC_VAR_NAME_MAX_LEN - 1);
    table->entries[2].value = calc_value_from_float(0.0);
    table->entries[2].is_const = false;

    table->count = 3;
}

bool calc_vars_get(const CalcVarTable *table, const char *name, CalcValue *out_val) {
    if (table == NULL || name == NULL) {
        return false;
    }
    for (size_t i = 0; i < table->count; ++i) {
        if (strcmp(table->entries[i].name, name) == 0) {
            if (out_val != NULL) {
                *out_val = table->entries[i].value;
            }
            return true;
        }
    }
    return false;
}

bool calc_vars_set(CalcVarTable *table, const char *name, const CalcValue *val, CalcError *err, CalcSpan span) {
    if (table == NULL || name == NULL || val == NULL) {
        return false;
    }

    if (strcmp(name, "ans") != 0 && calc_vars_is_reserved(name)) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_UNASSIGNABLE, span, "Cannot reassign reserved constant or function '%s'", name);
        }
        return false;
    }

    for (size_t i = 0; i < table->count; ++i) {
        if (strcmp(table->entries[i].name, name) == 0) {
            if (table->entries[i].is_const) {
                if (err != NULL) {
                    calc_error_set(err, CALC_ERR_UNASSIGNABLE, span, "Cannot modify constant variable '%s'", name);
                }
                return false;
            }
            table->entries[i].value = *val;
            return true;
        }
    }

    if (table->count >= CALC_VARS_MAX_ENTRIES) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_NOMEM, span, "Variable table limit reached (max %d)", CALC_VARS_MAX_ENTRIES);
        }
        return false;
    }

    CalcVarEntry *entry = &table->entries[table->count];
    strncpy(entry->name, name, CALC_VAR_NAME_MAX_LEN - 1);
    entry->name[CALC_VAR_NAME_MAX_LEN - 1] = '\0';
    entry->value = *val;
    entry->is_const = false;
    table->count++;

    return true;
}

void calc_vars_set_ans(CalcVarTable *table, const CalcValue *val) {
    if (table == NULL || val == NULL) {
        return;
    }
    for (size_t i = 0; i < table->count; ++i) {
        if (strcmp(table->entries[i].name, "ans") == 0) {
            table->entries[i].value = *val;
            return;
        }
    }
    if (table->count < CALC_VARS_MAX_ENTRIES) {
        CalcVarEntry *entry = &table->entries[table->count];
        strncpy(entry->name, "ans", CALC_VAR_NAME_MAX_LEN - 1);
        entry->name[CALC_VAR_NAME_MAX_LEN - 1] = '\0';
        entry->value = *val;
        entry->is_const = false;
        table->count++;
    }
}

size_t calc_vars_count(const CalcVarTable *table) {
    return table != NULL ? table->count : 0;
}

bool calc_vars_get_entry(const CalcVarTable *table, size_t index, const CalcVarEntry **out_entry) {
    if (table == NULL || out_entry == NULL || index >= table->count) {
        return false;
    }
    *out_entry = &table->entries[index];
    return true;
}
