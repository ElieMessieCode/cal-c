#include "test.h"
#include "calc/value.h"

TEST_BEGIN(test_gcd)
    ASSERT_EQ_INT(calc_gcd(12, 18), 6);
    ASSERT_EQ_INT(calc_gcd(101, 103), 1);
    ASSERT_EQ_INT(calc_gcd(-12, 18), 6);
    ASSERT_EQ_INT(calc_gcd(0, 5), 5);
    ASSERT_EQ_INT(calc_gcd(7, 0), 7);
TEST_END()

TEST_BEGIN(test_fraction_arithmetic)
    CalcFraction a = {1, 3};
    CalcFraction b = {1, 4};
    CalcFraction res;
    CalcError err;

    ASSERT_TRUE(calc_fraction_add(a, b, &res, &err, (CalcSpan){0, 0}));
    ASSERT_EQ_INT(res.num, 7);
    ASSERT_EQ_INT(res.den, 12);

    ASSERT_TRUE(calc_fraction_sub(a, b, &res, &err, (CalcSpan){0, 0}));
    ASSERT_EQ_INT(res.num, 1);
    ASSERT_EQ_INT(res.den, 12);

    ASSERT_TRUE(calc_fraction_mul(a, b, &res, &err, (CalcSpan){0, 0}));
    ASSERT_EQ_INT(res.num, 1);
    ASSERT_EQ_INT(res.den, 12);

    ASSERT_TRUE(calc_fraction_div(a, b, &res, &err, (CalcSpan){0, 0}));
    ASSERT_EQ_INT(res.num, 4);
    ASSERT_EQ_INT(res.den, 3);
TEST_END()

TEST_BEGIN(test_fraction_pow_and_factorial)
    CalcFraction a = {2, 3};
    CalcFraction res;
    CalcError err;

    ASSERT_TRUE(calc_fraction_pow(a, 3, &res, &err, (CalcSpan){0, 0}));
    ASSERT_EQ_INT(res.num, 8);
    ASSERT_EQ_INT(res.den, 27);

    ASSERT_TRUE(calc_fraction_pow(a, -2, &res, &err, (CalcSpan){0, 0}));
    ASSERT_EQ_INT(res.num, 9);
    ASSERT_EQ_INT(res.den, 4);

    CalcFraction f5 = {5, 1};
    ASSERT_TRUE(calc_fraction_factorial(f5, &res, &err, (CalcSpan){0, 0}));
    ASSERT_EQ_INT(res.num, 120);
    ASSERT_EQ_INT(res.den, 1);
TEST_END()

TEST_BEGIN(test_float_factorial)
    double res = 0.0;
    CalcError err;

    ASSERT_TRUE(calc_float_factorial(0.0, &res, &err, (CalcSpan){0, 0}));
    ASSERT_NEAR(res, 1.0, 1e-12);

    ASSERT_TRUE(calc_float_factorial(5.0, &res, &err, (CalcSpan){0, 0}));
    ASSERT_NEAR(res, 120.0, 1e-12);

    ASSERT_TRUE(calc_float_factorial(10.0, &res, &err, (CalcSpan){0, 0}));
    ASSERT_NEAR(res, 3628800.0, 1e-12);

    ASSERT_FALSE(calc_float_factorial(-1.0, &res, &err, (CalcSpan){0, 0}));
    ASSERT_FALSE(calc_float_factorial(171.0, &res, &err, (CalcSpan){0, 0}));
TEST_END()

TEST_BEGIN(test_value_format)
    char buf[64];
    CalcValue v1 = calc_value_from_float(3.14159);
    calc_value_format(&v1, 6, buf, sizeof(buf));
    ASSERT_EQ_STR(buf, "3.14159");

    CalcValue v2 = calc_value_from_exact((CalcFraction){42, 1});
    calc_value_format(&v2, 6, buf, sizeof(buf));
    ASSERT_EQ_STR(buf, "42");

    CalcValue v3 = calc_value_from_exact((CalcFraction){7, 12});
    calc_value_format(&v3, 4, buf, sizeof(buf));
    ASSERT_EQ_STR(buf, "7/12 (≈ 0.5833)");
TEST_END()

int main(void) {
    printf("Running test_value...\n");
    RUN_TEST(test_gcd);
    RUN_TEST(test_fraction_arithmetic);
    RUN_TEST(test_fraction_pow_and_factorial);
    RUN_TEST(test_float_factorial);
    RUN_TEST(test_value_format);
    TEST_REPORT();
}
