#include "test.h"
#include "calc/eval.h"
#include <stdio.h>
#include <string.h>

static CalcErrCode parse_errcode(const char *name) {
    if (strcmp(name, "CALC_ERR_SYNTAX") == 0) return CALC_ERR_SYNTAX;
    if (strcmp(name, "CALC_ERR_UNKNOWN_IDENT") == 0) return CALC_ERR_UNKNOWN_IDENT;
    if (strcmp(name, "CALC_ERR_ARITY") == 0) return CALC_ERR_ARITY;
    if (strcmp(name, "CALC_ERR_DIV_ZERO") == 0) return CALC_ERR_DIV_ZERO;
    if (strcmp(name, "CALC_ERR_DOMAIN") == 0) return CALC_ERR_DOMAIN;
    if (strcmp(name, "CALC_ERR_OVERFLOW") == 0) return CALC_ERR_OVERFLOW;
    if (strcmp(name, "CALC_ERR_TOO_DEEP") == 0) return CALC_ERR_TOO_DEEP;
    if (strcmp(name, "CALC_ERR_TOO_LONG") == 0) return CALC_ERR_TOO_LONG;
    if (strcmp(name, "CALC_ERR_NOMEM") == 0) return CALC_ERR_NOMEM;
    if (strcmp(name, "CALC_ERR_UNASSIGNABLE") == 0) return CALC_ERR_UNASSIGNABLE;
    if (strcmp(name, "CALC_ERR_NOT_AN_INTEGER") == 0) return CALC_ERR_NOT_AN_INTEGER;
    return CALC_ERR_NONE;
}

static FILE *open_cases_file(void) {
    const char *paths[] = {
        "tests/cases.tsv",
        "../tests/cases.tsv",
        "../../tests/cases.tsv",
        "cal-c/tests/cases.tsv",
        NULL
    };
    for (int i = 0; paths[i] != NULL; ++i) {
        FILE *f = fopen(paths[i], "r");
        if (f != NULL) return f;
    }
    return NULL;
}

TEST_BEGIN(test_all_tsv_cases)
    FILE *f = open_cases_file();
    ASSERT_TRUE(f != NULL);

    uint8_t arena_buf[32 * 1024];
    CalcArena arena;
    calc_arena_init(&arena, arena_buf, sizeof(arena_buf));

    CalcVarTable vars;
    calc_vars_init(&vars);

    char line[1024];
    int line_num = 0;
    int cases_tested = 0;

    while (fgets(line, sizeof(line), f) != NULL) {
        line_num++;
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        if (len == 0 || line[0] == '#') {
            continue;
        }

        char *expr = strtok(line, "\t");
        char *mode_str = strtok(NULL, "\t");
        char *type_str = strtok(NULL, "\t");
        char *val_str = strtok(NULL, "\t");

        if (expr == NULL || mode_str == NULL || type_str == NULL || val_str == NULL) {
            continue;
        }

        calc_arena_reset(&arena);
        CalcEvalConfig cfg;
        cfg.mode = (strcmp(mode_str, "exact") == 0) ? CALC_MODE_EXACT : CALC_MODE_FLOAT;
        cfg.precision = 15;

        CalcValue val;
        CalcError err;
        calc_error_init(&err);

        bool success = calc_eval_string(expr, strlen(expr), &vars, &cfg, &arena, &val, &err);
        cases_tested++;

        if (strcmp(type_str, "OK") == 0) {
            if (!success) {
                printf("    Line %d failed for '%s': unexpected error %s (%s)\n", line_num, expr, calc_errcode_str(err.code), err.msg);
                ASSERT_TRUE(success);
            }

            if (cfg.mode == CALC_MODE_FLOAT) {
                double expected = atof(val_str);
                double actual = (val.kind == CALC_VAL_FLOAT) ? val.v.fval : calc_fraction_to_double(val.v.exact);
                double diff = fabs(actual - expected);
                double tol = (fabs(expected) > 1.0) ? fabs(expected) * 1e-10 : 1e-10;
                if (diff > tol) {
                    printf("    Line %d mismatch for '%s': got %g, expected %g\n", line_num, expr, actual, expected);
                    ASSERT_TRUE(diff <= tol);
                }
            } else {
                int64_t exp_num = 0, exp_den = 1;
                char *slash = strchr(val_str, '/');
                if (slash != NULL) {
                    *slash = '\0';
                    exp_num = atoll(val_str);
                    exp_den = atoll(slash + 1);
                } else {
                    exp_num = atoll(val_str);
                    exp_den = 1;
                }

                if (val.kind != CALC_VAL_EXACT || val.v.exact.num != exp_num || val.v.exact.den != exp_den) {
                    printf("    Line %d exact mismatch for '%s': got %" PRId64 "/%" PRId64 ", expected %" PRId64 "/%" PRId64 "\n",
                           line_num, expr, val.v.exact.num, val.v.exact.den, exp_num, exp_den);
                    ASSERT_EQ_INT(val.v.exact.num, exp_num);
                    ASSERT_EQ_INT(val.v.exact.den, exp_den);
                }
            }
        } else {
            CalcErrCode expected_code = parse_errcode(type_str);
            if (success) {
                printf("    Line %d error expectation failed for '%s': expected error %s, got success\n", line_num, expr, type_str);
                ASSERT_FALSE(success);
            }
            if (err.code != expected_code) {
                printf("    Line %d error code mismatch for '%s': got %s, expected %s\n", line_num, expr, calc_errcode_str(err.code), type_str);
                ASSERT_EQ_INT(err.code, expected_code);
            }
        }
    }

    fclose(f);
    printf("    Cases validated from TSV: %d\n", cases_tested);
    ASSERT_TRUE(cases_tested >= 200);
TEST_END()

int main(void) {
    printf("Running test_cases (TSV test runner)...\n");
    RUN_TEST(test_all_tsv_cases);
    TEST_REPORT();
}
