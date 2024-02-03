#ifndef CALC_LEXER_H
#define CALC_LEXER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "calc/arena.h"
#include "calc/error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CalcTokenKind {
    TOK_EOF = 0,
    TOK_NUMBER,
    TOK_IDENT,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_PERCENT,
    TOK_CARET,
    TOK_BANG,
    TOK_ASSIGN,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_COMMA
} CalcTokenKind;

typedef struct CalcToken {
    CalcTokenKind kind;
    CalcSpan      span;
    bool          is_integer_literal;
    union {
        double      fval;
        int64_t     ival;
        const char *ident;
    } val;
} CalcToken;

typedef struct CalcLexer {
    const char *src;
    size_t      src_len;
    size_t      cursor;
    CalcArena  *arena;
    bool        has_peek;
    CalcToken   peek_tok;
} CalcLexer;

void calc_lexer_init(CalcLexer *lexer, const char *src, size_t src_len, CalcArena *arena);
bool calc_lexer_next(CalcLexer *lexer, CalcToken *tok, CalcError *err);
bool calc_lexer_peek(CalcLexer *lexer, CalcToken *tok, CalcError *err);
const char *calc_token_kind_str(CalcTokenKind kind);

#ifdef __cplusplus
}
#endif

#endif
