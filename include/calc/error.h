#ifndef CALC_ERROR_H
#define CALC_ERROR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CalcErrCode {
    CALC_ERR_NONE = 0,
    CALC_ERR_SYNTAX,
    CALC_ERR_UNKNOWN_IDENT,
    CALC_ERR_ARITY,
    CALC_ERR_DIV_ZERO,
    CALC_ERR_DOMAIN,
    CALC_ERR_OVERFLOW,
    CALC_ERR_TOO_DEEP,
    CALC_ERR_TOO_LONG,
    CALC_ERR_NOMEM,
    CALC_ERR_UNASSIGNABLE,
    CALC_ERR_NOT_AN_INTEGER
} CalcErrCode;

typedef struct CalcSpan {
    size_t start;
    size_t len;
} CalcSpan;

typedef struct CalcError {
    CalcErrCode code;
    CalcSpan    span;
    char        msg[96];
} CalcError;

void calc_error_init(CalcError *err);
void calc_error_set(CalcError *err, CalcErrCode code, CalcSpan span, const char *fmt, ...);
int calc_error_format_caret(const CalcError *err, const char *source, char *out_buf, size_t out_cap);
const char *calc_errcode_str(CalcErrCode code);

#ifdef __cplusplus
}
#endif

#endif
