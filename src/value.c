#include "calc/value.h"
#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include <fenv.h>
#include <float.h>

static inline bool safe_add_i64(int64_t a, int64_t b, int64_t *res) {
#if defined(__GNUC__) || defined(__clang__)
    return !__builtin_add_overflow(a, b, res);
#else
    if ((b > 0 && a > INT64_MAX - b) || (b < 0 && a < INT64_MIN - b)) {
        return false;
    }
    *res = a + b;
    return true;
#endif
}

static inline bool safe_sub_i64(int64_t a, int64_t b, int64_t *res) {
#if defined(__GNUC__) || defined(__clang__)
    return !__builtin_sub_overflow(a, b, res);
#else
    if ((b < 0 && a > INT64_MAX + b) || (b > 0 && a < INT64_MIN + b)) {
        return false;
    }
    *res = a - b;
    return true;
#endif
}

static inline bool safe_mul_i64(int64_t a, int64_t b, int64_t *res) {
#if defined(__GNUC__) || defined(__clang__)
    return !__builtin_mul_overflow(a, b, res);
#else
    if (a == 0 || b == 0) {
        *res = 0;
        return true;
    }
    if (a == INT64_MIN) {
        if (b == 1) {
            *res = INT64_MIN;
            return true;
        }
        return false;
    }
    if (b == INT64_MIN) {
        if (a == 1) {
            *res = INT64_MIN;
            return true;
        }
        return false;
    }
    if (a > 0) {
        if (b > 0) {
            if (a > INT64_MAX / b) return false;
        } else {
            if (b < INT64_MIN / a) return false;
        }
    } else {
        if (b > 0) {
            if (a < INT64_MIN / b) return false;
        } else {
            if (a < INT64_MAX / b) return false;
        }
    }
    *res = a * b;
    return true;
#endif
}

static inline bool safe_neg_i64(int64_t a, int64_t *res) {
    if (a == INT64_MIN) {
        return false;
    }
    *res = -a;
    return true;
}

int64_t calc_gcd(int64_t a, int64_t b) {
    uint64_t ua = (a < 0) ? (uint64_t)-(a + 1) + 1 : (uint64_t)a;
    uint64_t ub = (b < 0) ? (uint64_t)-(b + 1) + 1 : (uint64_t)b;

    while (ub != 0) {
        uint64_t temp = ua % ub;
        ua = ub;
        ub = temp;
    }
    return (int64_t)ua;
}

bool calc_fraction_normalize(int64_t num, int64_t den, CalcFraction *out, CalcError *err, CalcSpan span) {
    if (den == 0) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_DIV_ZERO, span, "Division by zero in rational arithmetic");
        }
        return false;
    }

    if (num == 0) {
        if (out != NULL) {
            out->num = 0;
            out->den = 1;
        }
        return true;
    }

    if (den < 0) {
        if (!safe_neg_i64(num, &num) || !safe_neg_i64(den, &den)) {
            if (err != NULL) {
                calc_error_set(err, CALC_ERR_OVERFLOW, span, "Integer overflow in fraction normalization");
            }
            return false;
        }
    }

    int64_t g = calc_gcd(num, den);
    if (g > 1) {
        num /= g;
        den /= g;
    }

    if (out != NULL) {
        out->num = num;
        out->den = den;
    }
    return true;
}

bool calc_fraction_add(CalcFraction a, CalcFraction b, CalcFraction *out, CalcError *err, CalcSpan span) {
    int64_t g = calc_gcd(a.den, b.den);
    int64_t d_over_g = b.den / g;
    int64_t b_over_g = a.den / g;

    int64_t term1 = 0, term2 = 0, num = 0, den = 0;
    if (!safe_mul_i64(a.num, d_over_g, &term1) ||
        !safe_mul_i64(b.num, b_over_g, &term2) ||
        !safe_add_i64(term1, term2, &num) ||
        !safe_mul_i64(b_over_g, b.den, &den)) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_OVERFLOW, span, "Integer overflow in fraction addition; use ':mode float'");
        }
        return false;
    }

    return calc_fraction_normalize(num, den, out, err, span);
}

bool calc_fraction_sub(CalcFraction a, CalcFraction b, CalcFraction *out, CalcError *err, CalcSpan span) {
    int64_t g = calc_gcd(a.den, b.den);
    int64_t d_over_g = b.den / g;
    int64_t b_over_g = a.den / g;

    int64_t term1 = 0, term2 = 0, num = 0, den = 0;
    if (!safe_mul_i64(a.num, d_over_g, &term1) ||
        !safe_mul_i64(b.num, b_over_g, &term2) ||
        !safe_sub_i64(term1, term2, &num) ||
        !safe_mul_i64(b_over_g, b.den, &den)) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_OVERFLOW, span, "Integer overflow in fraction subtraction; use ':mode float'");
        }
        return false;
    }

    return calc_fraction_normalize(num, den, out, err, span);
}

bool calc_fraction_mul(CalcFraction a, CalcFraction b, CalcFraction *out, CalcError *err, CalcSpan span) {
    int64_t g1 = calc_gcd(a.num, b.den);
    int64_t g2 = calc_gcd(b.num, a.den);

    int64_t a_num_red = a.num / g1;
    int64_t b_den_red = b.den / g1;
    int64_t b_num_red = b.num / g2;
    int64_t a_den_red = a.den / g2;

    int64_t num = 0, den = 0;
    if (!safe_mul_i64(a_num_red, b_num_red, &num) ||
        !safe_mul_i64(a_den_red, b_den_red, &den)) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_OVERFLOW, span, "Integer overflow in fraction multiplication; use ':mode float'");
        }
        return false;
    }

    return calc_fraction_normalize(num, den, out, err, span);
}

bool calc_fraction_div(CalcFraction a, CalcFraction b, CalcFraction *out, CalcError *err, CalcSpan span) {
    if (b.num == 0) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_DIV_ZERO, span, "Division by zero");
        }
        return false;
    }

    CalcFraction b_inv;
    b_inv.num = b.den;
    b_inv.den = b.num;
    return calc_fraction_mul(a, b_inv, out, err, span);
}

bool calc_fraction_pow(CalcFraction a, int64_t exp, CalcFraction *out, CalcError *err, CalcSpan span) {
    if (exp == 0) {
        if (a.num == 0) {
            if (err != NULL) {
                calc_error_set(err, CALC_ERR_DOMAIN, span, "0^0 is undefined in exact mode");
            }
            return false;
        }
        if (out != NULL) {
            out->num = 1;
            out->den = 1;
        }
        return true;
    }

    if (exp < 0) {
        if (a.num == 0) {
            if (err != NULL) {
                calc_error_set(err, CALC_ERR_DIV_ZERO, span, "Division by zero in negative exponent");
            }
            return false;
        }
        if (exp == INT64_MIN) {
            if (err != NULL) {
                calc_error_set(err, CALC_ERR_OVERFLOW, span, "Exponent overflow");
            }
            return false;
        }
        CalcFraction inv;
        inv.num = a.den;
        inv.den = a.num;
        return calc_fraction_pow(inv, -exp, out, err, span);
    }

    CalcFraction base = a;
    CalcFraction result = {1, 1};

    while (exp > 0) {
        if ((exp & 1) != 0) {
            if (!calc_fraction_mul(result, base, &result, err, span)) {
                return false;
            }
        }
        exp >>= 1;
        if (exp > 0) {
            if (!calc_fraction_mul(base, base, &base, err, span)) {
                return false;
            }
        }
    }

    if (out != NULL) {
        *out = result;
    }
    return true;
}

bool calc_fraction_neg(CalcFraction a, CalcFraction *out, CalcError *err, CalcSpan span) {
    int64_t num = 0;
    if (!safe_neg_i64(a.num, &num)) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_OVERFLOW, span, "Integer overflow in negation");
        }
        return false;
    }
    if (out != NULL) {
        out->num = num;
        out->den = a.den;
    }
    return true;
}

bool calc_fraction_abs(CalcFraction a, CalcFraction *out, CalcError *err, CalcSpan span) {
    if (a.num >= 0) {
        if (out != NULL) {
            *out = a;
        }
        return true;
    }
    return calc_fraction_neg(a, out, err, span);
}

bool calc_fraction_mod(CalcFraction a, CalcFraction b, CalcFraction *out, CalcError *err, CalcSpan span) {
    if (a.den != 1 || b.den != 1) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_NOT_AN_INTEGER, span, "Modulo operator '%' requires integer operands in exact mode");
        }
        return false;
    }
    if (b.num == 0) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_DIV_ZERO, span, "Division by zero in modulo");
        }
        return false;
    }
    if (a.num == INT64_MIN && b.num == -1) {
        if (out != NULL) {
            out->num = 0;
            out->den = 1;
        }
        return true;
    }
    if (out != NULL) {
        out->num = a.num % b.num;
        out->den = 1;
    }
    return true;
}

bool calc_fraction_min(CalcFraction a, CalcFraction b, CalcFraction *out) {
    CalcFraction diff;
    if (calc_fraction_sub(a, b, &diff, NULL, (CalcSpan){0, 0})) {
        if (diff.num <= 0) {
            if (out != NULL) *out = a;
        } else {
            if (out != NULL) *out = b;
        }
    } else {
        double da = calc_fraction_to_double(a);
        double db = calc_fraction_to_double(b);
        if (out != NULL) *out = (da <= db) ? a : b;
    }
    return true;
}

bool calc_fraction_max(CalcFraction a, CalcFraction b, CalcFraction *out) {
    CalcFraction diff;
    if (calc_fraction_sub(a, b, &diff, NULL, (CalcSpan){0, 0})) {
        if (diff.num >= 0) {
            if (out != NULL) *out = a;
        } else {
            if (out != NULL) *out = b;
        }
    } else {
        double da = calc_fraction_to_double(a);
        double db = calc_fraction_to_double(b);
        if (out != NULL) *out = (da >= db) ? a : b;
    }
    return true;
}

bool calc_fraction_factorial(CalcFraction a, CalcFraction *out, CalcError *err, CalcSpan span) {
    if (a.den != 1 || a.num < 0) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_NOT_AN_INTEGER, span, "Factorial requires a non-negative integer");
        }
        return false;
    }
    if (a.num > 20) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_OVERFLOW, span, "Exact factorial overflow: %" PRId64 "! exceeds 64-bit int; use ':mode float'", a.num);
        }
        return false;
    }

    int64_t res = 1;
    for (int64_t i = 2; i <= a.num; ++i) {
        res *= i;
    }
    if (out != NULL) {
        out->num = res;
        out->den = 1;
    }
    return true;
}

double calc_fraction_to_double(CalcFraction frac) {
    if (frac.den == 0) return 0.0;
    return (double)frac.num / (double)frac.den;
}

bool calc_float_check(double val, CalcError *err, CalcSpan span) {
    if (isnan(val)) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_DOMAIN, span, "Domain error (result is NaN)");
        }
        return false;
    }
    if (isinf(val)) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_OVERFLOW, span, "Arithmetic overflow / infinity");
        }
        return false;
    }
    return true;
}

bool calc_float_factorial(double n, double *out, CalcError *err, CalcSpan span) {
    if (n < 0.0 || floor(n) != n) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_DOMAIN, span, "Factorial requires a non-negative integer (got %g)", n);
        }
        return false;
    }
    if (n > 170.0) {
        if (err != NULL) {
            calc_error_set(err, CALC_ERR_OVERFLOW, span, "Factorial overflow: %g! exceeds double capacity (max 170)", n);
        }
        return false;
    }

    double res = 1.0;
    int k = (int)n;
    for (int i = 2; i <= k; ++i) {
        res *= (double)i;
    }
    if (out != NULL) {
        *out = res;
    }
    return true;
}

CalcValue calc_value_from_float(double fval) {
    CalcValue v;
    v.kind = CALC_VAL_FLOAT;
    v.v.fval = fval;
    return v;
}

CalcValue calc_value_from_exact(CalcFraction frac) {
    CalcValue v;
    v.kind = CALC_VAL_EXACT;
    v.v.exact = frac;
    return v;
}

int calc_value_format(const CalcValue *val, int precision, char *buf, size_t buf_size) {
    if (val == NULL || buf == NULL || buf_size == 0) {
        return 0;
    }

    if (val->kind == CALC_VAL_FLOAT) {
        return snprintf(buf, buf_size, "%.*g", precision, val->v.fval);
    }

    if (val->v.exact.den == 1) {
        return snprintf(buf, buf_size, "%" PRId64, val->v.exact.num);
    }

    double dval = calc_fraction_to_double(val->v.exact);
    return snprintf(
        buf,
        buf_size,
        "%" PRId64 "/%" PRId64 " (≈ %.*g)",
        val->v.exact.num,
        val->v.exact.den,
        precision,
        dval
    );
}
