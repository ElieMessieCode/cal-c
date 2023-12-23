#ifndef CALC_VALUE_H
#define CALC_VALUE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "calc/error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CalcFraction {
    int64_t num;
    int64_t den;
} CalcFraction;

typedef enum CalcValueKind {
    CALC_VAL_FLOAT,
    CALC_VAL_EXACT
} CalcValueKind;

typedef struct CalcValue {
    CalcValueKind kind;
    union {
        double       fval;
        CalcFraction exact;
    } v;
} CalcValue;

int64_t calc_gcd(int64_t a, int64_t b);
bool calc_fraction_normalize(int64_t num, int64_t den, CalcFraction *out, CalcError *err, CalcSpan span);
bool calc_fraction_add(CalcFraction a, CalcFraction b, CalcFraction *out, CalcError *err, CalcSpan span);
bool calc_fraction_sub(CalcFraction a, CalcFraction b, CalcFraction *out, CalcError *err, CalcSpan span);
bool calc_fraction_mul(CalcFraction a, CalcFraction b, CalcFraction *out, CalcError *err, CalcSpan span);
bool calc_fraction_div(CalcFraction a, CalcFraction b, CalcFraction *out, CalcError *err, CalcSpan span);
bool calc_fraction_pow(CalcFraction a, int64_t exp, CalcFraction *out, CalcError *err, CalcSpan span);
bool calc_fraction_neg(CalcFraction a, CalcFraction *out, CalcError *err, CalcSpan span);
bool calc_fraction_abs(CalcFraction a, CalcFraction *out, CalcError *err, CalcSpan span);
bool calc_fraction_mod(CalcFraction a, CalcFraction b, CalcFraction *out, CalcError *err, CalcSpan span);
bool calc_fraction_min(CalcFraction a, CalcFraction b, CalcFraction *out);
bool calc_fraction_max(CalcFraction a, CalcFraction b, CalcFraction *out);
bool calc_fraction_factorial(CalcFraction a, CalcFraction *out, CalcError *err, CalcSpan span);
double calc_fraction_to_double(CalcFraction frac);

bool calc_float_check(double val, CalcError *err, CalcSpan span);
bool calc_float_factorial(double n, double *out, CalcError *err, CalcSpan span);

CalcValue calc_value_from_float(double fval);
CalcValue calc_value_from_exact(CalcFraction frac);
int calc_value_format(const CalcValue *val, int precision, char *buf, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif
