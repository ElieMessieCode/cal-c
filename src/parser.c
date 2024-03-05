#include "calc/parser.h"
#include "calc/lexer.h"
#include <string.h>

static CalcAstNode *alloc_ast_node(CalcArena *arena, CalcAstKind kind, CalcSpan span) {
    CalcAstNode *node = (CalcAstNode *)calc_arena_alloc(arena, sizeof(CalcAstNode), _Alignof(CalcAstNode));
    if (node != NULL) {
        node->kind = kind;
        node->span = span;
    }
    return node;
}

static CalcAstNode *parse_expr(CalcLexer *lexer, CalcArena *arena, size_t depth, CalcError *err);
static CalcAstNode *parse_unary(CalcLexer *lexer, CalcArena *arena, size_t depth, CalcError *err);

static CalcAstNode *parse_primary(CalcLexer *lexer, CalcArena *arena, size_t depth, CalcError *err) {
    if (depth > CALC_AST_MAX_DEPTH) {
        calc_error_set(err, CALC_ERR_TOO_DEEP, (CalcSpan){lexer->cursor, 1}, "Expression nesting exceeds maximum depth of %d", CALC_AST_MAX_DEPTH);
        return NULL;
    }

    CalcToken tok;
    if (!calc_lexer_peek(lexer, &tok, err)) {
        return NULL;
    }

    if (tok.kind == TOK_NUMBER) {
        calc_lexer_next(lexer, &tok, err);
        CalcAstNode *node = alloc_ast_node(arena, AST_NUMBER, tok.span);
        if (node == NULL) {
            calc_error_set(err, CALC_ERR_NOMEM, tok.span, "Memory arena out of space");
            return NULL;
        }

        if (tok.is_integer_literal) {
            node->as.number.val = calc_value_from_exact((CalcFraction){tok.val.ival, 1});
            node->as.number.is_exact_integer = true;
        } else {
            node->as.number.val = calc_value_from_float(tok.val.fval);
            node->as.number.is_exact_integer = false;
        }
        return node;
    }

    if (tok.kind == TOK_IDENT) {
        calc_lexer_next(lexer, &tok, err);
        CalcSpan ident_span = tok.span;
        const char *ident_name = tok.val.ident;

        CalcToken next_tok;
        if (!calc_lexer_peek(lexer, &next_tok, err)) {
            return NULL;
        }

        if (next_tok.kind == TOK_LPAREN) {
            calc_lexer_next(lexer, &next_tok, err); // consume '('

            CalcAstNode *call_node = alloc_ast_node(arena, AST_CALL, ident_span);
            if (call_node == NULL) {
                calc_error_set(err, CALC_ERR_NOMEM, ident_span, "Memory arena out of space");
                return NULL;
            }
            call_node->as.call.name = ident_name;
            call_node->as.call.arg_count = 0;

            CalcToken peek_arg;
            if (!calc_lexer_peek(lexer, &peek_arg, err)) {
                return NULL;
            }

            if (peek_arg.kind != TOK_RPAREN) {
                while (true) {
                    if (call_node->as.call.arg_count >= CALC_MAX_CALL_ARGS) {
                        calc_error_set(err, CALC_ERR_ARITY, peek_arg.span, "Too many arguments in function call (max %d)", CALC_MAX_CALL_ARGS);
                        return NULL;
                    }

                    CalcAstNode *arg = parse_expr(lexer, arena, depth + 1, err);
                    if (arg == NULL) {
                        return NULL;
                    }

                    call_node->as.call.args[call_node->as.call.arg_count++] = arg;

                    CalcToken sep;
                    if (!calc_lexer_peek(lexer, &sep, err)) {
                        return NULL;
                    }

                    if (sep.kind == TOK_COMMA) {
                        calc_lexer_next(lexer, &sep, err);
                    } else if (sep.kind == TOK_RPAREN) {
                        break;
                    } else {
                        calc_error_set(err, CALC_ERR_SYNTAX, sep.span, "Expected ',' or ')' in argument list, got %s", calc_token_kind_str(sep.kind));
                        return NULL;
                    }
                }
            }

            CalcToken close_paren;
            if (!calc_lexer_next(lexer, &close_paren, err)) {
                return NULL;
            }
            if (close_paren.kind != TOK_RPAREN) {
                calc_error_set(err, CALC_ERR_SYNTAX, close_paren.span, "Expected ')' to close argument list");
                return NULL;
            }

            call_node->span.len = (close_paren.span.start + close_paren.span.len) - ident_span.start;
            return call_node;
        }

        CalcAstNode *node = alloc_ast_node(arena, AST_IDENT, ident_span);
        if (node == NULL) {
            calc_error_set(err, CALC_ERR_NOMEM, ident_span, "Memory arena out of space");
            return NULL;
        }
        node->as.ident.name = ident_name;
        return node;
    }

    if (tok.kind == TOK_LPAREN) {
        calc_lexer_next(lexer, &tok, err);
        CalcSpan lparen_span = tok.span;

        CalcAstNode *inner = parse_expr(lexer, arena, depth + 1, err);
        if (inner == NULL) {
            return NULL;
        }

        CalcToken rparen;
        if (!calc_lexer_next(lexer, &rparen, err)) {
            return NULL;
        }
        if (rparen.kind != TOK_RPAREN) {
            calc_error_set(err, CALC_ERR_SYNTAX, rparen.span, "Expected closing ')'");
            return NULL;
        }

        inner->span.start = lparen_span.start;
        inner->span.len = (rparen.span.start + rparen.span.len) - lparen_span.start;
        return inner;
    }

    calc_error_set(err, CALC_ERR_SYNTAX, tok.span, "Expected expression, got %s", calc_token_kind_str(tok.kind));
    return NULL;
}

static CalcAstNode *parse_postfix(CalcLexer *lexer, CalcArena *arena, size_t depth, CalcError *err) {
    CalcAstNode *operand = parse_primary(lexer, arena, depth, err);
    if (operand == NULL) {
        return NULL;
    }

    while (true) {
        CalcToken tok;
        if (!calc_lexer_peek(lexer, &tok, err)) {
            return NULL;
        }

        if (tok.kind == TOK_BANG) {
            calc_lexer_next(lexer, &tok, err);
            CalcSpan span = {operand->span.start, (tok.span.start + tok.span.len) - operand->span.start};
            CalcAstNode *node = alloc_ast_node(arena, AST_POSTFIX, span);
            if (node == NULL) {
                calc_error_set(err, CALC_ERR_NOMEM, tok.span, "Memory arena out of space");
                return NULL;
            }
            node->as.postfix.op = OP_POSTFIX_FACTORIAL;
            node->as.postfix.operand = operand;
            operand = node;
        } else {
            break;
        }
    }

    return operand;
}

static CalcAstNode *parse_power(CalcLexer *lexer, CalcArena *arena, size_t depth, CalcError *err) {
    if (depth > CALC_AST_MAX_DEPTH) {
        calc_error_set(err, CALC_ERR_TOO_DEEP, (CalcSpan){lexer->cursor, 1}, "Expression nesting exceeds maximum depth of %d", CALC_AST_MAX_DEPTH);
        return NULL;
    }

    CalcAstNode *left = parse_postfix(lexer, arena, depth, err);
    if (left == NULL) {
        return NULL;
    }

    CalcToken tok;
    if (!calc_lexer_peek(lexer, &tok, err)) {
        return NULL;
    }

    if (tok.kind == TOK_CARET) {
        calc_lexer_next(lexer, &tok, err);
        CalcAstNode *right = parse_unary(lexer, arena, depth + 1, err);
        if (right == NULL) {
            return NULL;
        }

        CalcSpan span = {left->span.start, (right->span.start + right->span.len) - left->span.start};
        CalcAstNode *node = alloc_ast_node(arena, AST_BINARY, span);
        if (node == NULL) {
            calc_error_set(err, CALC_ERR_NOMEM, tok.span, "Memory arena out of space");
            return NULL;
        }
        node->as.binary.op = OP_POW;
        node->as.binary.left = left;
        node->as.binary.right = right;
        return node;
    }

    return left;
}

static CalcAstNode *parse_unary(CalcLexer *lexer, CalcArena *arena, size_t depth, CalcError *err) {
    if (depth > CALC_AST_MAX_DEPTH) {
        calc_error_set(err, CALC_ERR_TOO_DEEP, (CalcSpan){lexer->cursor, 1}, "Expression nesting exceeds maximum depth of %d", CALC_AST_MAX_DEPTH);
        return NULL;
    }

    CalcToken tok;
    if (!calc_lexer_peek(lexer, &tok, err)) {
        return NULL;
    }

    if (tok.kind == TOK_PLUS || tok.kind == TOK_MINUS) {
        calc_lexer_next(lexer, &tok, err);
        CalcSpan op_span = tok.span;
        CalcUnaryOp op = (tok.kind == TOK_PLUS) ? OP_UNARY_PLUS : OP_UNARY_MINUS;

        CalcAstNode *operand = parse_unary(lexer, arena, depth + 1, err);
        if (operand == NULL) {
            return NULL;
        }

        CalcSpan span = {op_span.start, (operand->span.start + operand->span.len) - op_span.start};
        CalcAstNode *node = alloc_ast_node(arena, AST_UNARY, span);
        if (node == NULL) {
            calc_error_set(err, CALC_ERR_NOMEM, op_span, "Memory arena out of space");
            return NULL;
        }
        node->as.unary.op = op;
        node->as.unary.operand = operand;
        return node;
    }

    return parse_power(lexer, arena, depth, err);
}

static CalcAstNode *parse_term(CalcLexer *lexer, CalcArena *arena, size_t depth, CalcError *err) {
    if (depth > CALC_AST_MAX_DEPTH) {
        calc_error_set(err, CALC_ERR_TOO_DEEP, (CalcSpan){lexer->cursor, 1}, "Expression nesting exceeds maximum depth of %d", CALC_AST_MAX_DEPTH);
        return NULL;
    }

    CalcAstNode *left = parse_unary(lexer, arena, depth, err);
    if (left == NULL) {
        return NULL;
    }

    while (true) {
        CalcToken tok;
        if (!calc_lexer_peek(lexer, &tok, err)) {
            return NULL;
        }

        if (tok.kind == TOK_STAR || tok.kind == TOK_SLASH || tok.kind == TOK_PERCENT) {
            calc_lexer_next(lexer, &tok, err);
            CalcBinaryOp op = (tok.kind == TOK_STAR) ? OP_MUL : ((tok.kind == TOK_SLASH) ? OP_DIV : OP_MOD);

            CalcAstNode *right = parse_unary(lexer, arena, depth + 1, err);
            if (right == NULL) {
                return NULL;
            }

            CalcSpan span = {left->span.start, (right->span.start + right->span.len) - left->span.start};
            CalcAstNode *node = alloc_ast_node(arena, AST_BINARY, span);
            if (node == NULL) {
                calc_error_set(err, CALC_ERR_NOMEM, tok.span, "Memory arena out of space");
                return NULL;
            }
            node->as.binary.op = op;
            node->as.binary.left = left;
            node->as.binary.right = right;
            left = node;
        } else {
            break;
        }
    }

    return left;
}

static CalcAstNode *parse_expr(CalcLexer *lexer, CalcArena *arena, size_t depth, CalcError *err) {
    if (depth > CALC_AST_MAX_DEPTH) {
        calc_error_set(err, CALC_ERR_TOO_DEEP, (CalcSpan){lexer->cursor, 1}, "Expression nesting exceeds maximum depth of %d", CALC_AST_MAX_DEPTH);
        return NULL;
    }

    CalcAstNode *left = parse_term(lexer, arena, depth, err);
    if (left == NULL) {
        return NULL;
    }

    while (true) {
        CalcToken tok;
        if (!calc_lexer_peek(lexer, &tok, err)) {
            return NULL;
        }

        if (tok.kind == TOK_PLUS || tok.kind == TOK_MINUS) {
            calc_lexer_next(lexer, &tok, err);
            CalcBinaryOp op = (tok.kind == TOK_PLUS) ? OP_ADD : OP_SUB;

            CalcAstNode *right = parse_term(lexer, arena, depth + 1, err);
            if (right == NULL) {
                return NULL;
            }

            CalcSpan span = {left->span.start, (right->span.start + right->span.len) - left->span.start};
            CalcAstNode *node = alloc_ast_node(arena, AST_BINARY, span);
            if (node == NULL) {
                calc_error_set(err, CALC_ERR_NOMEM, tok.span, "Memory arena out of space");
                return NULL;
            }
            node->as.binary.op = op;
            node->as.binary.left = left;
            node->as.binary.right = right;
            left = node;
        } else {
            break;
        }
    }

    return left;
}

static CalcAstNode *parse_stmt(CalcLexer *lexer, CalcArena *arena, CalcError *err) {
    CalcToken tok1;
    if (!calc_lexer_peek(lexer, &tok1, err)) {
        return NULL;
    }

    if (tok1.kind == TOK_IDENT) {
        CalcToken consumed_tok1;
        calc_lexer_next(lexer, &consumed_tok1, err);

        CalcToken tok2;
        if (!calc_lexer_peek(lexer, &tok2, err)) {
            return NULL;
        }

        if (tok2.kind == TOK_ASSIGN) {
            calc_lexer_next(lexer, &tok2, err); // consume '='
            CalcAstNode *expr = parse_expr(lexer, arena, 1, err);
            if (expr == NULL) {
                return NULL;
            }

            CalcSpan span = {consumed_tok1.span.start, (expr->span.start + expr->span.len) - consumed_tok1.span.start};
            CalcAstNode *node = alloc_ast_node(arena, AST_ASSIGN, span);
            if (node == NULL) {
                calc_error_set(err, CALC_ERR_NOMEM, consumed_tok1.span, "Memory arena out of space");
                return NULL;
            }
            node->as.assign.name = consumed_tok1.val.ident;
            node->as.assign.expr = expr;
            return node;
        }
    }

    // Reset cursor to 0 and re-parse expr
    lexer->cursor = 0;
    lexer->has_peek = false;

    return parse_expr(lexer, arena, 1, err);
}

bool calc_parse(const char *src, size_t len, CalcArena *arena, CalcAstNode **out_ast, CalcError *err) {
    if (src == NULL || arena == NULL || out_ast == NULL) {
        return false;
    }

    if (len > CALC_MAX_INPUT_LEN) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_TOO_LONG, (CalcSpan){0, len}, "Expression length (%zu) exceeds limit of %d bytes", len, CALC_MAX_INPUT_LEN);
        }
        return false;
    }

    CalcLexer lexer;
    calc_lexer_init(&lexer, src, len, arena);

    CalcToken first_tok;
    if (!calc_lexer_peek(&lexer, &first_tok, err)) {
        return false;
    }
    if (first_tok.kind == TOK_EOF) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_SYNTAX, (CalcSpan){0, 0}, "Empty expression");
        }
        return false;
    }

    CalcAstNode *ast = parse_stmt(&lexer, arena, err);
    if (ast == NULL) {
        return false;
    }

    CalcToken trailing;
    if (!calc_lexer_next(&lexer, &trailing, err)) {
        return false;
    }
    if (trailing.kind != TOK_EOF) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_SYNTAX, trailing.span, "Unexpected token after expression: %s", calc_token_kind_str(trailing.kind));
        }
        return false;
    }

    *out_ast = ast;
    return true;
}
