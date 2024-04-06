#include "test.h"
#include "calc/eval.h"

TEST_BEGIN(test_eval_float_arithmetic)
    uint8_t buf[16 * 1024];
    CalcArena arena;
    calc_arena_init(&arena, buf, sizeof(buf));
    CalcVarTable vars;
    calc_vars_init(&vars);
    CalcEvalConfig cfg = calc_eval_config_default();
    CalcValue val;
    CalcError err;

    // 2 + 3 * 4 = 14
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_eval_string("2 + 3 * 4", 9, &vars, &cfg, &arena, &val, &err));
    ASSERT_NEAR(val.v.fval, 14.0, 1e-12);

    // 2 ^ 3 ^ 2 = 512
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_eval_string("2 ^ 3 ^ 2", 9, &vars, &cfg, &arena, &val, &err));
    ASSERT_NEAR(val.v.fval, 512.0, 1e-12);

    // -2 ^ 2 = -4
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_eval_string("-2 ^ 2", 6, &vars, &cfg, &arena, &val, &err));
    ASSERT_NEAR(val.v.fval, -4.0, 1e-12);

    // 2 ^ -2 = 0.25
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_eval_string("2 ^ -2", 6, &vars, &cfg, &arena, &val, &err));
    ASSERT_NEAR(val.v.fval, 0.25, 1e-12);

    // 5! = 120
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_eval_string("5!", 2, &vars, &cfg, &arena, &val, &err));
    ASSERT_NEAR(val.v.fval, 120.0, 1e-12);
TEST_END()

TEST_BEGIN(test_eval_functions_and_vars)
    uint8_t buf[16 * 1024];
    CalcArena arena;
    calc_arena_init(&arena, buf, sizeof(buf));
    CalcVarTable vars;
    calc_vars_init(&vars);
    CalcEvalConfig cfg = calc_eval_config_default();
    CalcValue val;
    CalcError err;

    // sin(pi / 2) = 1
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_eval_string("sin(pi / 2)", 11, &vars, &cfg, &arena, &val, &err));
    ASSERT_NEAR(val.v.fval, 1.0, 1e-12);

    // sqrt(16) = 4
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_eval_string("sqrt(16)", 8, &vars, &cfg, &arena, &val, &err));
    ASSERT_NEAR(val.v.fval, 4.0, 1e-12);

    // min(10, 20) = 10
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_eval_string("min(10, 20)", 11, &vars, &cfg, &arena, &val, &err));
    ASSERT_NEAR(val.v.fval, 10.0, 1e-12);

    // Variable assignment and use: a = 3 * 4; a + 2 = 14
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_eval_string("a = 3 * 4", 9, &vars, &cfg, &arena, &val, &err));
    ASSERT_NEAR(val.v.fval, 12.0, 1e-12);

    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_eval_string("a + 2", 5, &vars, &cfg, &arena, &val, &err));
    ASSERT_NEAR(val.v.fval, 14.0, 1e-12);

    // ans variable check
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_eval_string("ans / 2", 7, &vars, &cfg, &arena, &val, &err));
    ASSERT_NEAR(val.v.fval, 7.0, 1e-12);
TEST_END()

TEST_BEGIN(test_eval_exact_mode)
    uint8_t buf[16 * 1024];
    CalcArena arena;
    calc_arena_init(&arena, buf, sizeof(buf));
    CalcVarTable vars;
    calc_vars_init(&vars);
    CalcEvalConfig cfg = {CALC_MODE_EXACT, 15};
    CalcValue val;
    CalcError err;

    // 1/3 + 1/4 = 7/12
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_eval_string("1/3 + 1/4", 9, &vars, &cfg, &arena, &val, &err));
    ASSERT_EQ_INT(val.kind, CALC_VAL_EXACT);
    ASSERT_EQ_INT(val.v.exact.num, 7);
    ASSERT_EQ_INT(val.v.exact.den, 12);

    // (2/3)^3 = 8/27
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_eval_string("(2/3)^3", 7, &vars, &cfg, &arena, &val, &err));
    ASSERT_EQ_INT(val.v.exact.num, 8);
    ASSERT_EQ_INT(val.v.exact.den, 27);

    // sqrt(9/16) = 3/4
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_eval_string("sqrt(9/16)", 10, &vars, &cfg, &arena, &val, &err));
    ASSERT_EQ_INT(val.v.exact.num, 3);
    ASSERT_EQ_INT(val.v.exact.den, 4);
TEST_END()

TEST_BEGIN(test_eval_errors)
    uint8_t buf[16 * 1024];
    CalcArena arena;
    calc_arena_init(&arena, buf, sizeof(buf));
    CalcVarTable vars;
    calc_vars_init(&vars);
    CalcEvalConfig cfg = calc_eval_config_default();
    CalcValue val;
    CalcError err;

    // Division by zero
    calc_arena_reset(&arena);
    ASSERT_FALSE(calc_eval_string("1 / 0", 5, &vars, &cfg, &arena, &val, &err));
    ASSERT_EQ_INT(err.code, CALC_ERR_DIV_ZERO);

    // Domain error (sqrt(-1))
    calc_arena_reset(&arena);
    ASSERT_FALSE(calc_eval_string("sqrt(-1)", 8, &vars, &cfg, &arena, &val, &err));
    ASSERT_EQ_INT(err.code, CALC_ERR_DOMAIN);

    // Arity mismatch
    calc_arena_reset(&arena);
    ASSERT_FALSE(calc_eval_string("sin(1, 2)", 9, &vars, &cfg, &arena, &val, &err));
    ASSERT_EQ_INT(err.code, CALC_ERR_ARITY);

    // Unknown identifier
    calc_arena_reset(&arena);
    ASSERT_FALSE(calc_eval_string("unknown_var + 1", 15, &vars, &cfg, &arena, &val, &err));
    ASSERT_EQ_INT(err.code, CALC_ERR_UNKNOWN_IDENT);
TEST_END()

int main(void) {
    printf("Running test_eval...\n");
    RUN_TEST(test_eval_float_arithmetic);
    RUN_TEST(test_eval_functions_and_vars);
    RUN_TEST(test_eval_exact_mode);
    RUN_TEST(test_eval_errors);
    TEST_REPORT();
}
