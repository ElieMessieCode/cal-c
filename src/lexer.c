#include "calc/lexer.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void calc_lexer_init(CalcLexer *lexer, const char *src, size_t src_len, CalcArena *arena) {
    if (lexer == NULL) {
        return;
    }
    lexer->src = src;
    lexer->src_len = src_len;
    lexer->cursor = 0;
    lexer->arena = arena;
    lexer->has_peek = false;
    memset(&lexer->peek_tok, 0, sizeof(lexer->peek_tok));
}

const char *calc_token_kind_str(CalcTokenKind kind) {
    switch (kind) {
        case TOK_EOF:     return "end of input";
        case TOK_NUMBER:  return "number";
        case TOK_IDENT:   return "identifier";
        case TOK_PLUS:    return "'+'";
        case TOK_MINUS:   return "'-'";
        case TOK_STAR:    return "'*'";
        case TOK_SLASH:   return "'/'";
        case TOK_PERCENT: return "'%'";
        case TOK_CARET:   return "'^'";
        case TOK_BANG:    return "'!'";
        case TOK_ASSIGN:  return "'='";
        case TOK_LPAREN:  return "'('";
        case TOK_RPAREN:  return "')'";
        case TOK_COMMA:   return "','";
        default:          return "token";
    }
}

static void skip_whitespace(CalcLexer *lexer) {
    while (lexer->cursor < lexer->src_len) {
        char c = lexer->src[lexer->cursor];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            lexer->cursor++;
        } else {
            break;
        }
    }
}

static bool lex_number(CalcLexer *lexer, CalcToken *tok, CalcError *err) {
    size_t start = lexer->cursor;
    const char *src = lexer->src;
    size_t len = lexer->src_len;

    if (src[start] == '0' && start + 1 < len) {
        char next = src[start + 1];
        if (next == 'x' || next == 'X') {
            lexer->cursor += 2;
            uint64_t val = 0;
            size_t digits = 0;

            while (lexer->cursor < len) {
                char c = src[lexer->cursor];
                if (c == '_') {
                    lexer->cursor++;
                    continue;
                }
                int digit = -1;
                if (c >= '0' && c <= '9') digit = c - '0';
                else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
                else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
                else break;

                val = (val << 4) | (uint64_t)digit;
                digits++;
                lexer->cursor++;
            }

            if (digits == 0) {
                if (err != NULL) {
                    calc_error_set(err, CALC_ERR_SYNTAX, (CalcSpan){start, lexer->cursor - start}, "Hexadecimal literal missing digits after 0x");
                }
                return false;
            }

            tok->kind = TOK_NUMBER;
            tok->span = (CalcSpan){start, lexer->cursor - start};
            tok->is_integer_literal = true;
            tok->val.ival = (int64_t)val;
            return true;
        }

        if (next == 'b' || next == 'B') {
            lexer->cursor += 2;
            uint64_t val = 0;
            size_t digits = 0;

            while (lexer->cursor < len) {
                char c = src[lexer->cursor];
                if (c == '_') {
                    lexer->cursor++;
                    continue;
                }
                if (c == '0' || c == '1') {
                    val = (val << 1) | (uint64_t)(c - '0');
                    digits++;
                    lexer->cursor++;
                } else {
                    break;
                }
            }

            if (digits == 0) {
                if (err != NULL) {
                    calc_error_set(err, CALC_ERR_SYNTAX, (CalcSpan){start, lexer->cursor - start}, "Binary literal missing digits after 0b");
                }
                return false;
            }

            tok->kind = TOK_NUMBER;
            tok->span = (CalcSpan){start, lexer->cursor - start};
            tok->is_integer_literal = true;
            tok->val.ival = (int64_t)val;
            return true;
        }
    }

    char clean_buf[128];
    size_t clean_pos = 0;
    bool has_dot = false;
    bool has_exp = false;

    while (lexer->cursor < len) {
        char c = src[lexer->cursor];
        if (c == '_') {
            lexer->cursor++;
            continue;
        }

        if (c == '.') {
            if (has_dot || has_exp) {
                break;
            }
            has_dot = true;
            if (clean_pos + 1 < sizeof(clean_buf)) {
                clean_buf[clean_pos++] = c;
            }
            lexer->cursor++;
            continue;
        }

        if (c == 'e' || c == 'E') {
            if (has_exp) {
                break;
            }
            has_exp = true;
            if (clean_pos + 1 < sizeof(clean_buf)) {
                clean_buf[clean_pos++] = c;
            }
            lexer->cursor++;
            if (lexer->cursor < len && (src[lexer->cursor] == '+' || src[lexer->cursor] == '-')) {
                if (clean_pos + 1 < sizeof(clean_buf)) {
                    clean_buf[clean_pos++] = src[lexer->cursor];
                }
                lexer->cursor++;
            }
            continue;
        }

        if (c >= '0' && c <= '9') {
            if (clean_pos + 1 < sizeof(clean_buf)) {
                clean_buf[clean_pos++] = c;
            }
            lexer->cursor++;
            continue;
        }

        break;
    }

    clean_buf[clean_pos] = '\0';
    tok->kind = TOK_NUMBER;
    tok->span = (CalcSpan){start, lexer->cursor - start};

    if (!has_dot && !has_exp) {
        errno = 0;
        char *endptr = NULL;
        long long ival = strtoll(clean_buf, &endptr, 10);
        if (errno == ERANGE || (endptr != NULL && *endptr != '\0')) {
            double dval = strtod(clean_buf, NULL);
            tok->is_integer_literal = false;
            tok->val.fval = dval;
        } else {
            tok->is_integer_literal = true;
            tok->val.ival = (int64_t)ival;
        }
    } else {
        char *endptr = NULL;
        double dval = strtod(clean_buf, &endptr);
        tok->is_integer_literal = false;
        tok->val.fval = dval;
    }

    return true;
}

static bool lex_ident(CalcLexer *lexer, CalcToken *tok, CalcError *err) {
    (void)err;
    size_t start = lexer->cursor;
    const char *src = lexer->src;
    size_t len = lexer->src_len;

    while (lexer->cursor < len) {
        char c = src[lexer->cursor];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') {
            lexer->cursor++;
        } else {
            break;
        }
    }

    size_t id_len = lexer->cursor - start;
    tok->kind = TOK_IDENT;
    tok->span = (CalcSpan){start, id_len};
    tok->is_integer_literal = false;
    tok->val.ident = calc_arena_strndup(lexer->arena, &src[start], id_len);

    return true;
}

static bool fetch_next_token(CalcLexer *lexer, CalcToken *tok, CalcError *err) {
    skip_whitespace(lexer);

    if (lexer->cursor >= lexer->src_len) {
        tok->kind = TOK_EOF;
        tok->span = (CalcSpan){lexer->src_len, 0};
        tok->is_integer_literal = false;
        tok->val.ival = 0;
        return true;
    }

    size_t start = lexer->cursor;
    char c = lexer->src[start];

    if (c >= '0' && c <= '9') {
        return lex_number(lexer, tok, err);
    }
    if (c == '.' && start + 1 < lexer->src_len && (lexer->src[start + 1] >= '0' && lexer->src[start + 1] <= '9')) {
        return lex_number(lexer, tok, err);
    }

    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        return lex_ident(lexer, tok, err);
    }

    lexer->cursor++;
    tok->span = (CalcSpan){start, 1};
    tok->is_integer_literal = false;

    switch (c) {
        case '+': tok->kind = TOK_PLUS; return true;
        case '-': tok->kind = TOK_MINUS; return true;
        case '*': tok->kind = TOK_STAR; return true;
        case '/': tok->kind = TOK_SLASH; return true;
        case '%': tok->kind = TOK_PERCENT; return true;
        case '^': tok->kind = TOK_CARET; return true;
        case '!': tok->kind = TOK_BANG; return true;
        case '=': tok->kind = TOK_ASSIGN; return true;
        case '(': tok->kind = TOK_LPAREN; return true;
        case ')': tok->kind = TOK_RPAREN; return true;
        case ',': tok->kind = TOK_COMMA; return true;
        default:
            if (err != NULL) {
                calc_error_set(err, CALC_ERR_SYNTAX, (CalcSpan){start, 1}, "Unexpected character '%c'", c);
            }
            return false;
    }
}

bool calc_lexer_next(CalcLexer *lexer, CalcToken *tok, CalcError *err) {
    if (lexer == NULL || tok == NULL) {
        return false;
    }

    if (lexer->has_peek) {
        *tok = lexer->peek_tok;
        lexer->has_peek = false;
        return true;
    }

    return fetch_next_token(lexer, tok, err);
}

bool calc_lexer_peek(CalcLexer *lexer, CalcToken *tok, CalcError *err) {
    if (lexer == NULL || tok == NULL) {
        return false;
    }

    if (!lexer->has_peek) {
        if (!fetch_next_token(lexer, &lexer->peek_tok, err)) {
            return false;
        }
        lexer->has_peek = true;
    }

    *tok = lexer->peek_tok;
    return true;
}
