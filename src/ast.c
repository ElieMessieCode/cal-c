#include "calc/ast.h"
#include <stdio.h>
#include <string.h>

const char *calc_binary_op_str(CalcBinaryOp op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_MOD: return "%";
        case OP_POW: return "^";
        default:     return "?";
    }
}

const char *calc_unary_op_str(CalcUnaryOp op) {
    switch (op) {
        case OP_UNARY_PLUS:  return "+";
        case OP_UNARY_MINUS: return "-";
        default:             return "?";
    }
}

static int format_node(const CalcAstNode *node, char *buf, size_t cap, size_t *offset) {
    if (node == NULL || *offset >= cap) {
        return 0;
    }

    int written = 0;

    switch (node->kind) {
        case AST_NUMBER: {
            char num_str[64];
            calc_value_format(&node->as.number.val, 15, num_str, sizeof(num_str));
            written = snprintf(buf + *offset, cap - *offset, "%s", num_str);
            if (written > 0) *offset += (size_t)written;
            break;
        }

        case AST_IDENT: {
            written = snprintf(buf + *offset, cap - *offset, "%s", node->as.ident.name);
            if (written > 0) *offset += (size_t)written;
            break;
        }

        case AST_UNARY: {
            written = snprintf(buf + *offset, cap - *offset, "(%s ", calc_unary_op_str(node->as.unary.op));
            if (written > 0) *offset += (size_t)written;

            format_node(node->as.unary.operand, buf, cap, offset);

            written = snprintf(buf + *offset, cap - *offset, ")");
            if (written > 0) *offset += (size_t)written;
            break;
        }

        case AST_BINARY: {
            written = snprintf(buf + *offset, cap - *offset, "(%s ", calc_binary_op_str(node->as.binary.op));
            if (written > 0) *offset += (size_t)written;

            format_node(node->as.binary.left, buf, cap, offset);

            written = snprintf(buf + *offset, cap - *offset, " ");
            if (written > 0) *offset += (size_t)written;

            format_node(node->as.binary.right, buf, cap, offset);

            written = snprintf(buf + *offset, cap - *offset, ")");
            if (written > 0) *offset += (size_t)written;
            break;
        }

        case AST_POSTFIX: {
            written = snprintf(buf + *offset, cap - *offset, "(! ");
            if (written > 0) *offset += (size_t)written;

            format_node(node->as.postfix.operand, buf, cap, offset);

            written = snprintf(buf + *offset, cap - *offset, ")");
            if (written > 0) *offset += (size_t)written;
            break;
        }

        case AST_CALL: {
            written = snprintf(buf + *offset, cap - *offset, "(%s", node->as.call.name);
            if (written > 0) *offset += (size_t)written;

            for (size_t i = 0; i < node->as.call.arg_count; ++i) {
                written = snprintf(buf + *offset, cap - *offset, " ");
                if (written > 0) *offset += (size_t)written;

                format_node(node->as.call.args[i], buf, cap, offset);
            }

            written = snprintf(buf + *offset, cap - *offset, ")");
            if (written > 0) *offset += (size_t)written;
            break;
        }

        case AST_ASSIGN: {
            written = snprintf(buf + *offset, cap - *offset, "(= %s ", node->as.assign.name);
            if (written > 0) *offset += (size_t)written;

            format_node(node->as.assign.expr, buf, cap, offset);

            written = snprintf(buf + *offset, cap - *offset, ")");
            if (written > 0) *offset += (size_t)written;
            break;
        }
    }

    return (int)*offset;
}

int calc_ast_format(const CalcAstNode *node, char *buf, size_t buf_size) {
    if (node == NULL || buf == NULL || buf_size == 0) {
        return 0;
    }

    size_t offset = 0;
    format_node(node, buf, buf_size, &offset);
    if (offset >= buf_size) {
        buf[buf_size - 1] = '\0';
    } else {
        buf[offset] = '\0';
    }
    return (int)offset;
}
