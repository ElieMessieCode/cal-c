#include "calc/error.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void calc_error_init(CalcError *err) {
    if (err == NULL) {
        return;
    }
    err->code = CALC_ERR_NONE;
    err->span.start = 0;
    err->span.len = 0;
    err->msg[0] = '\0';
}

void calc_error_set(CalcError *err, CalcErrCode code, CalcSpan span, const char *fmt, ...) {
    if (err == NULL) {
        return;
    }
    err->code = code;
    err->span = span;
    if (fmt == NULL) {
        err->msg[0] = '\0';
        return;
    }
    va_list args;
    va_start(args, fmt);
    vsnprintf(err->msg, sizeof(err->msg), fmt, args);
    va_end(args);
}

const char *calc_errcode_str(CalcErrCode code) {
    switch (code) {
        case CALC_ERR_NONE:
            return "No error";
        case CALC_ERR_SYNTAX:
            return "Syntax error";
        case CALC_ERR_UNKNOWN_IDENT:
            return "Unknown identifier";
        case CALC_ERR_ARITY:
            return "Arity mismatch";
        case CALC_ERR_DIV_ZERO:
            return "Division by zero";
        case CALC_ERR_DOMAIN:
            return "Domain error";
        case CALC_ERR_OVERFLOW:
            return "Arithmetic overflow";
        case CALC_ERR_TOO_DEEP:
            return "Expression nesting too deep";
        case CALC_ERR_TOO_LONG:
            return "Expression too long";
        case CALC_ERR_NOMEM:
            return "Out of memory";
        case CALC_ERR_UNASSIGNABLE:
            return "Unassignable target";
        case CALC_ERR_NOT_AN_INTEGER:
            return "Integer required";
        default:
            return "Unknown error";
    }
}

int calc_error_format_caret(const CalcError *err, const char *source, char *out_buf, size_t out_cap) {
    if (err == NULL || out_buf == NULL || out_cap == 0) {
        return 0;
    }

    if (err->code == CALC_ERR_NONE) {
        out_buf[0] = '\0';
        return 0;
    }

    const char *src = (source != NULL) ? source : "";
    size_t src_len = strlen(src);
    size_t start = err->span.start;
    size_t len = err->span.len;

    if (start > src_len) {
        start = src_len;
    }
    if (len == 0) {
        len = 1;
    }

    char caret_line[256];
    size_t pos = 0;

    for (size_t i = 0; i < start && pos + 1 < sizeof(caret_line); ++i) {
        caret_line[pos++] = (src[i] == '\t') ? '\t' : ' ';
    }
    for (size_t i = 0; i < len && pos + 1 < sizeof(caret_line); ++i) {
        caret_line[pos++] = '^';
    }
    caret_line[pos] = '\0';

    int written = snprintf(
        out_buf,
        out_cap,
        "error: %s\n  %s\n  %s",
        err->msg[0] != '\0' ? err->msg : calc_errcode_str(err->code),
        src,
        caret_line
    );

    return written;
}
