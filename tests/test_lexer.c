#include "test.h"
#include "calc/lexer.h"

TEST_BEGIN(test_lexer_numbers)
    uint8_t buf[1024];
    CalcArena arena;
    calc_arena_init(&arena, buf, sizeof(buf));

    const char *src = "123 45.67 1.2e3 0xFF 0b1010 1_000_000";
    CalcLexer lex;
    calc_lexer_init(&lex, src, strlen(src), &arena);
    CalcToken tok;
    CalcError err;

    // 123
    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_NUMBER);
    ASSERT_TRUE(tok.is_integer_literal);
    ASSERT_EQ_INT(tok.val.ival, 123);

    // 45.67
    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_NUMBER);
    ASSERT_FALSE(tok.is_integer_literal);
    ASSERT_NEAR(tok.val.fval, 45.67, 1e-12);

    // 1.2e3
    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_NUMBER);
    ASSERT_FALSE(tok.is_integer_literal);
    ASSERT_NEAR(tok.val.fval, 1200.0, 1e-12);

    // 0xFF
    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_NUMBER);
    ASSERT_TRUE(tok.is_integer_literal);
    ASSERT_EQ_INT(tok.val.ival, 255);

    // 0b1010
    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_NUMBER);
    ASSERT_TRUE(tok.is_integer_literal);
    ASSERT_EQ_INT(tok.val.ival, 10);

    // 1_000_000
    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_NUMBER);
    ASSERT_TRUE(tok.is_integer_literal);
    ASSERT_EQ_INT(tok.val.ival, 1000000);

    // EOF
    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_EOF);
TEST_END()

TEST_BEGIN(test_lexer_tokens)
    uint8_t buf[1024];
    CalcArena arena;
    calc_arena_init(&arena, buf, sizeof(buf));

    const char *src = "x = sin(pi * 2) + 3^2!";
    CalcLexer lex;
    calc_lexer_init(&lex, src, strlen(src), &arena);
    CalcToken tok;
    CalcError err;

    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_IDENT);
    ASSERT_EQ_STR(tok.val.ident, "x");

    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_ASSIGN);

    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_IDENT);
    ASSERT_EQ_STR(tok.val.ident, "sin");

    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_LPAREN);

    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_IDENT);
    ASSERT_EQ_STR(tok.val.ident, "pi");

    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_STAR);

    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_NUMBER);

    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_RPAREN);

    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_PLUS);

    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_NUMBER);

    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_CARET);

    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_NUMBER);

    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_BANG);

    ASSERT_TRUE(calc_lexer_next(&lex, &tok, &err));
    ASSERT_EQ_INT(tok.kind, TOK_EOF);
TEST_END()

int main(void) {
    printf("Running test_lexer...\n");
    RUN_TEST(test_lexer_numbers);
    RUN_TEST(test_lexer_tokens);
    TEST_REPORT();
}
