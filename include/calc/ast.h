#ifndef CALC_AST_H
#define CALC_AST_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "calc/arena.h"
#include "calc/error.h"
#include "calc/value.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CALC_AST_MAX_DEPTH 256
#define CALC_MAX_CALL_ARGS 16

typedef enum CalcAstKind {
    AST_NUMBER = 0,
    AST_IDENT,
    AST_UNARY,
    AST_BINARY,
    AST_POSTFIX,
    AST_CALL,
    AST_ASSIGN
} CalcAstKind;

typedef enum CalcBinaryOp {
    OP_ADD = 0,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_POW
} CalcBinaryOp;

typedef enum CalcUnaryOp {
    OP_UNARY_PLUS = 0,
    OP_UNARY_MINUS
} CalcUnaryOp;

typedef enum CalcPostfixOp {
    OP_POSTFIX_FACTORIAL = 0
} CalcPostfixOp;

typedef struct CalcAstNode CalcAstNode;

struct CalcAstNode {
    CalcAstKind kind;
    CalcSpan    span;
    union {
        struct {
            CalcValue val;
            bool      is_exact_integer;
        } number;

        struct {
            const char *name;
        } ident;

        struct {
            CalcUnaryOp        op;
            const CalcAstNode *operand;
        } unary;

        struct {
            CalcBinaryOp       op;
            const CalcAstNode *left;
            const CalcAstNode *right;
        } binary;

        struct {
            CalcPostfixOp      op;
            const CalcAstNode *operand;
        } postfix;

        struct {
            const char        *name;
            const CalcAstNode *args[CALC_MAX_CALL_ARGS];
            size_t             arg_count;
        } call;

        struct {
            const char        *name;
            const CalcAstNode *expr;
        } assign;
    } as;
};

int calc_ast_format(const CalcAstNode *node, char *buf, size_t buf_size);
const char *calc_binary_op_str(CalcBinaryOp op);
const char *calc_unary_op_str(CalcUnaryOp op);

#ifdef __cplusplus
}
#endif

#endif
