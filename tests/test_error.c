#include "test.h"
#include "calc/error.h"

TEST_BEGIN(test_error_formatting)
    CalcError err;
    calc_error_init(&err);

    ASSERT_EQ_INT(err.code, CALC_ERR_NONE);

    calc_error_set(&err, CALC_ERR_SYNTAX, (CalcSpan){4, 2}, "unexpected token '%s'", "*");
    ASSERT_EQ_INT(err.code, CALC_ERR_SYNTAX);
    ASSERT_EQ_STR(err.msg, "unexpected token '*'");

    char out[256];
    const char *src = "2 + * 3";
    calc_error_format_caret(&err, src, out, sizeof(out));

    const char *expected = "error: unexpected token '*'\n  2 + * 3\n      ^^";
    ASSERT_EQ_STR(out, expected);
TEST_END()

int main(void) {
    printf("Running test_error...\n");
    RUN_TEST(test_error_formatting);
    TEST_REPORT();
}
