#include "test.h"
#include "calc/vars.h"

TEST_BEGIN(test_vars_init)
    CalcVarTable table;
    calc_vars_init(&table);

    ASSERT_EQ_INT(calc_vars_count(&table), 3);

    CalcValue val;
    ASSERT_TRUE(calc_vars_get(&table, "pi", &val));
    ASSERT_NEAR(val.v.fval, 3.141592653589793, 1e-12);

    ASSERT_TRUE(calc_vars_get(&table, "e", &val));
    ASSERT_NEAR(val.v.fval, 2.718281828459045, 1e-12);

    ASSERT_TRUE(calc_vars_get(&table, "ans", &val));
    ASSERT_NEAR(val.v.fval, 0.0, 1e-12);
TEST_END()

TEST_BEGIN(test_vars_set_and_get)
    CalcVarTable table;
    calc_vars_init(&table);
    CalcError err;

    CalcValue v10 = calc_value_from_float(10.5);
    ASSERT_TRUE(calc_vars_set(&table, "x", &v10, &err, (CalcSpan){0, 0}));

    CalcValue out;
    ASSERT_TRUE(calc_vars_get(&table, "x", &out));
    ASSERT_NEAR(out.v.fval, 10.5, 1e-12);

    // Constant protection
    CalcValue v_bad = calc_value_from_float(4.0);
    ASSERT_FALSE(calc_vars_set(&table, "pi", &v_bad, &err, (CalcSpan){0, 0}));
    ASSERT_EQ_INT(err.code, CALC_ERR_UNASSIGNABLE);

    // Reserved function name protection
    ASSERT_FALSE(calc_vars_set(&table, "sin", &v_bad, &err, (CalcSpan){0, 0}));
    ASSERT_EQ_INT(err.code, CALC_ERR_UNASSIGNABLE);

    // ans update
    CalcValue v_ans = calc_value_from_float(42.0);
    calc_vars_set_ans(&table, &v_ans);
    ASSERT_TRUE(calc_vars_get(&table, "ans", &out));
    ASSERT_NEAR(out.v.fval, 42.0, 1e-12);
TEST_END()

int main(void) {
    printf("Running test_vars...\n");
    RUN_TEST(test_vars_init);
    RUN_TEST(test_vars_set_and_get);
    TEST_REPORT();
}
