#include "test.h"
#include "calc/parser.h"

TEST_BEGIN(test_parser_precedence_and_ast)
    uint8_t buf[16 * 1024];
    CalcArena arena;
    calc_arena_init(&arena, buf, sizeof(buf));
    CalcAstNode *ast = NULL;
    CalcError err;
    char out[256];

    // Precedence: 2 + 3 * 4 -> (+ 2 (* 3 4))
    const char *s1 = "2 + 3 * 4";
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_parse(s1, strlen(s1), &arena, &ast, &err));
    calc_ast_format(ast, out, sizeof(out));
    ASSERT_EQ_STR(out, "(+ 2 (* 3 4))");

    // Right-associativity of power: 2 ^ 3 ^ 2 -> (^ 2 (^ 3 2))
    const char *s2 = "2 ^ 3 ^ 2";
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_parse(s2, strlen(s2), &arena, &ast, &err));
    calc_ast_format(ast, out, sizeof(out));
    ASSERT_EQ_STR(out, "(^ 2 (^ 3 2))");

    // Unary minus with power: -2 ^ 2 -> (- (^ 2 2))
    const char *s3 = "-2 ^ 2";
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_parse(s3, strlen(s3), &arena, &ast, &err));
    calc_ast_format(ast, out, sizeof(out));
    ASSERT_EQ_STR(out, "(- (^ 2 2))");

    // Power with negative exponent: 2 ^ -2 -> (^ 2 (- 2))
    const char *s4 = "2 ^ -2";
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_parse(s4, strlen(s4), &arena, &ast, &err));
    calc_ast_format(ast, out, sizeof(out));
    ASSERT_EQ_STR(out, "(^ 2 (- 2))");

    // Postfix factorial: 3!! -> (! (! 3))
    const char *s5 = "3!!";
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_parse(s5, strlen(s5), &arena, &ast, &err));
    calc_ast_format(ast, out, sizeof(out));
    ASSERT_EQ_STR(out, "(! (! 3))");

    // Function call: min(1, 2)
    const char *s6 = "min(1, 2)";
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_parse(s6, strlen(s6), &arena, &ast, &err));
    calc_ast_format(ast, out, sizeof(out));
    ASSERT_EQ_STR(out, "(min 1 2)");

    // Assignment: x = 10 + 5
    const char *s7 = "x = 10 + 5";
    calc_arena_reset(&arena);
    ASSERT_TRUE(calc_parse(s7, strlen(s7), &arena, &ast, &err));
    calc_ast_format(ast, out, sizeof(out));
    ASSERT_EQ_STR(out, "(= x (+ 10 5))");
TEST_END()

TEST_BEGIN(test_parser_errors)
    uint8_t buf[16 * 1024];
    CalcArena arena;
    calc_arena_init(&arena, buf, sizeof(buf));
    CalcAstNode *ast = NULL;
    CalcError err;

    // Unexpected trailing token
    const char *s1 = "2 + 3 4";
    calc_arena_reset(&arena);
    ASSERT_FALSE(calc_parse(s1, strlen(s1), &arena, &ast, &err));
    ASSERT_EQ_INT(err.code, CALC_ERR_SYNTAX);

    // Unclosed parenthesis
    const char *s2 = "(2 + 3";
    calc_arena_reset(&arena);
    ASSERT_FALSE(calc_parse(s2, strlen(s2), &arena, &ast, &err));
    ASSERT_EQ_INT(err.code, CALC_ERR_SYNTAX);

    // Empty expression
    const char *s3 = "   ";
    calc_arena_reset(&arena);
    ASSERT_FALSE(calc_parse(s3, strlen(s3), &arena, &ast, &err));
    ASSERT_EQ_INT(err.code, CALC_ERR_SYNTAX);
TEST_END()

int main(void) {
    printf("Running test_parser...\n");
    RUN_TEST(test_parser_precedence_and_ast);
    RUN_TEST(test_parser_errors);
    TEST_REPORT();
}
