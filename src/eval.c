#include "calc/eval.h"
#include "calc/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fenv.h>
#include <inttypes.h>

CalcEvalConfig calc_eval_config_default(void) {
    CalcEvalConfig cfg;
    cfg.mode = CALC_MODE_FLOAT;
    cfg.precision = CALC_DEFAULT_PRECISION;
    return cfg;
}

static bool eval_call_float(const CalcAstNode *ast, const double *arg_vals, double *out, CalcError *err) {
    const char *name = ast->as.call.name;
    size_t count = ast->as.call.arg_count;
    CalcSpan span = ast->span;

#if defined(__STDC_VERSION__) && !defined(_MSC_VER)
    #pragma STDC FENV_ACCESS ON
#endif
    feclearexcept(FE_ALL_EXCEPT);

    // 1-arg functions
    if (strcmp(name, "sin") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'sin' expects 1 argument (got %zu)", count);
            return false;
        }
        *out = sin(arg_vals[0]);
    } else if (strcmp(name, "cos") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'cos' expects 1 argument (got %zu)", count);
            return false;
        }
        *out = cos(arg_vals[0]);
    } else if (strcmp(name, "tan") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'tan' expects 1 argument (got %zu)", count);
            return false;
        }
        *out = tan(arg_vals[0]);
    } else if (strcmp(name, "asin") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'asin' expects 1 argument (got %zu)", count);
            return false;
        }
        if (arg_vals[0] < -1.0 || arg_vals[0] > 1.0) {
            calc_error_set(err, CALC_ERR_DOMAIN, span, "Domain error: asin argument must be in [-1, 1] (got %g)", arg_vals[0]);
            return false;
        }
        *out = asin(arg_vals[0]);
    } else if (strcmp(name, "acos") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'acos' expects 1 argument (got %zu)", count);
            return false;
        }
        if (arg_vals[0] < -1.0 || arg_vals[0] > 1.0) {
            calc_error_set(err, CALC_ERR_DOMAIN, span, "Domain error: acos argument must be in [-1, 1] (got %g)", arg_vals[0]);
            return false;
        }
        *out = acos(arg_vals[0]);
    } else if (strcmp(name, "atan") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'atan' expects 1 argument (got %zu)", count);
            return false;
        }
        *out = atan(arg_vals[0]);
    } else if (strcmp(name, "sqrt") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'sqrt' expects 1 argument (got %zu)", count);
            return false;
        }
        if (arg_vals[0] < 0.0) {
            calc_error_set(err, CALC_ERR_DOMAIN, span, "Domain error: sqrt argument must be non-negative (got %g)", arg_vals[0]);
            return false;
        }
        *out = sqrt(arg_vals[0]);
    } else if (strcmp(name, "cbrt") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'cbrt' expects 1 argument (got %zu)", count);
            return false;
        }
        *out = cbrt(arg_vals[0]);
    } else if (strcmp(name, "exp") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'exp' expects 1 argument (got %zu)", count);
            return false;
        }
        *out = exp(arg_vals[0]);
    } else if (strcmp(name, "ln") == 0 || strcmp(name, "log") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'ln' expects 1 argument (got %zu)", count);
            return false;
        }
        if (arg_vals[0] <= 0.0) {
            calc_error_set(err, CALC_ERR_DOMAIN, span, "Domain error: ln argument must be strictly positive (got %g)", arg_vals[0]);
            return false;
        }
        *out = log(arg_vals[0]);
    } else if (strcmp(name, "log10") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'log10' expects 1 argument (got %zu)", count);
            return false;
        }
        if (arg_vals[0] <= 0.0) {
            calc_error_set(err, CALC_ERR_DOMAIN, span, "Domain error: log10 argument must be strictly positive (got %g)", arg_vals[0]);
            return false;
        }
        *out = log10(arg_vals[0]);
    } else if (strcmp(name, "abs") == 0 || strcmp(name, "fabs") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'abs' expects 1 argument (got %zu)", count);
            return false;
        }
        *out = fabs(arg_vals[0]);
    } else if (strcmp(name, "floor") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'floor' expects 1 argument (got %zu)", count);
            return false;
        }
        *out = floor(arg_vals[0]);
    } else if (strcmp(name, "ceil") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'ceil' expects 1 argument (got %zu)", count);
            return false;
        }
        *out = ceil(arg_vals[0]);
    } else if (strcmp(name, "round") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'round' expects 1 argument (got %zu)", count);
            return false;
        }
        *out = round(arg_vals[0]);
    }
    // 2-arg functions
    else if (strcmp(name, "min") == 0) {
        if (count != 2) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'min' expects 2 arguments (got %zu)", count);
            return false;
        }
        *out = (arg_vals[0] <= arg_vals[1]) ? arg_vals[0] : arg_vals[1];
    } else if (strcmp(name, "max") == 0) {
        if (count != 2) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'max' expects 2 arguments (got %zu)", count);
            return false;
        }
        *out = (arg_vals[0] >= arg_vals[1]) ? arg_vals[0] : arg_vals[1];
    } else if (strcmp(name, "pow") == 0) {
        if (count != 2) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'pow' expects 2 arguments (got %zu)", count);
            return false;
        }
        if (arg_vals[0] == 0.0 && arg_vals[1] < 0.0) {
            calc_error_set(err, CALC_ERR_DIV_ZERO, span, "Division by zero (0 raised to negative power)");
            return false;
        }
        if (arg_vals[0] < 0.0 && floor(arg_vals[1]) != arg_vals[1]) {
            calc_error_set(err, CALC_ERR_DOMAIN, span, "Domain error: negative base with non-integer exponent");
            return false;
        }
        *out = pow(arg_vals[0], arg_vals[1]);
    } else if (strcmp(name, "hypot") == 0) {
        if (count != 2) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'hypot' expects 2 arguments (got %zu)", count);
            return false;
        }
        *out = hypot(arg_vals[0], arg_vals[1]);
    } else {
        calc_error_set(err, CALC_ERR_UNKNOWN_IDENT, span, "Unknown function '%s'", name);
        return false;
    }

    return calc_float_check(*out, err, span);
}

static int64_t is_perfect_square(int64_t n) {
    if (n < 0) return -1;
    if (n == 0) return 0;
    int64_t r = (int64_t)sqrt((double)n);
    while ((r + 1) * (r + 1) <= n) r++;
    while (r * r > n) r--;
    return (r * r == n) ? r : -1;
}

static bool eval_call_exact(const CalcAstNode *ast, const CalcFraction *arg_fracs, CalcFraction *out, CalcError *err) {
    const char *name = ast->as.call.name;
    size_t count = ast->as.call.arg_count;
    CalcSpan span = ast->span;

    if (strcmp(name, "abs") == 0 || strcmp(name, "fabs") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'abs' expects 1 argument (got %zu)", count);
            return false;
        }
        return calc_fraction_abs(arg_fracs[0], out, err, span);
    }

    if (strcmp(name, "min") == 0) {
        if (count != 2) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'min' expects 2 arguments (got %zu)", count);
            return false;
        }
        return calc_fraction_min(arg_fracs[0], arg_fracs[1], out);
    }

    if (strcmp(name, "max") == 0) {
        if (count != 2) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'max' expects 2 arguments (got %zu)", count);
            return false;
        }
        return calc_fraction_max(arg_fracs[0], arg_fracs[1], out);
    }

    if (strcmp(name, "pow") == 0) {
        if (count != 2) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'pow' expects 2 arguments (got %zu)", count);
            return false;
        }
        if (arg_fracs[1].den != 1) {
            calc_error_set(err, CALC_ERR_NOT_AN_INTEGER, span, "Exact mode pow requires integer exponent; switch to ':mode float'");
            return false;
        }
        return calc_fraction_pow(arg_fracs[0], arg_fracs[1].num, out, err, span);
    }

    if (strcmp(name, "sqrt") == 0) {
        if (count != 1) {
            calc_error_set(err, CALC_ERR_ARITY, span, "Function 'sqrt' expects 1 argument (got %zu)", count);
            return false;
        }
        if (arg_fracs[0].num < 0) {
            calc_error_set(err, CALC_ERR_DOMAIN, span, "Domain error: sqrt argument must be non-negative");
            return false;
        }
        int64_t sqrt_num = is_perfect_square(arg_fracs[0].num);
        int64_t sqrt_den = is_perfect_square(arg_fracs[0].den);
        if (sqrt_num >= 0 && sqrt_den > 0) {
            return calc_fraction_normalize(sqrt_num, sqrt_den, out, err, span);
        }
        calc_error_set(err, CALC_ERR_DOMAIN, span, "Irreducible square root in exact mode; use ':mode float'");
        return false;
    }

    // Check if known math function
    if (calc_vars_is_reserved(name)) {
        calc_error_set(err, CALC_ERR_DOMAIN, span, "Function '%s' is not supported in exact mode; use ':mode float'", name);
        return false;
    }

    calc_error_set(err, CALC_ERR_UNKNOWN_IDENT, span, "Unknown function '%s'", name);
    return false;
}

static bool eval_node_float(const CalcAstNode *ast, CalcVarTable *vars, const CalcEvalConfig *config, double *out_f, CalcError *err) {
    if (ast == NULL) return false;
    (void)config;

    switch (ast->kind) {
        case AST_NUMBER: {
            if (ast->as.number.val.kind == CALC_VAL_FLOAT) {
                *out_f = ast->as.number.val.v.fval;
            } else {
                *out_f = calc_fraction_to_double(ast->as.number.val.v.exact);
            }
            return true;
        }

        case AST_IDENT: {
            CalcValue val;
            if (!calc_vars_get(vars, ast->as.ident.name, &val)) {
                calc_error_set(err, CALC_ERR_UNKNOWN_IDENT, ast->span, "Unknown variable '%s'", ast->as.ident.name);
                return false;
            }
            if (val.kind == CALC_VAL_FLOAT) {
                *out_f = val.v.fval;
            } else {
                *out_f = calc_fraction_to_double(val.v.exact);
            }
            return true;
        }

        case AST_UNARY: {
            double operand = 0.0;
            if (!eval_node_float(ast->as.unary.operand, vars, config, &operand, err)) {
                return false;
            }
            if (ast->as.unary.op == OP_UNARY_PLUS) {
                *out_f = +operand;
            } else {
                *out_f = -operand;
            }
            return calc_float_check(*out_f, err, ast->span);
        }

        case AST_POSTFIX: {
            double operand = 0.0;
            if (!eval_node_float(ast->as.postfix.operand, vars, config, &operand, err)) {
                return false;
            }
            if (ast->as.postfix.op == OP_POSTFIX_FACTORIAL) {
                return calc_float_factorial(operand, out_f, err, ast->span);
            }
            return false;
        }

        case AST_BINARY: {
            double left = 0.0, right = 0.0;
            if (!eval_node_float(ast->as.binary.left, vars, config, &left, err)) {
                return false;
            }
            if (!eval_node_float(ast->as.binary.right, vars, config, &right, err)) {
                return false;
            }

#if defined(__STDC_VERSION__) && !defined(_MSC_VER)
            #pragma STDC FENV_ACCESS ON
#endif
            feclearexcept(FE_ALL_EXCEPT);

            switch (ast->as.binary.op) {
                case OP_ADD:
                    *out_f = left + right;
                    break;
                case OP_SUB:
                    *out_f = left - right;
                    break;
                case OP_MUL:
                    *out_f = left * right;
                    break;
                case OP_DIV:
                    if (right == 0.0) {
                        calc_error_set(err, CALC_ERR_DIV_ZERO, ast->span, "Division by zero");
                        return false;
                    }
                    *out_f = left / right;
                    break;
                case OP_MOD:
                    if (right == 0.0) {
                        calc_error_set(err, CALC_ERR_DIV_ZERO, ast->span, "Division by zero in modulo");
                        return false;
                    }
                    *out_f = fmod(left, right);
                    break;
                case OP_POW:
                    if (left == 0.0 && right < 0.0) {
                        calc_error_set(err, CALC_ERR_DIV_ZERO, ast->span, "Division by zero (0 raised to negative power)");
                        return false;
                    }
                    if (left < 0.0 && floor(right) != right) {
                        calc_error_set(err, CALC_ERR_DOMAIN, ast->span, "Domain error: negative base with non-integer exponent");
                        return false;
                    }
                    *out_f = pow(left, right);
                    break;
            }

            return calc_float_check(*out_f, err, ast->span);
        }

        case AST_CALL: {
            double arg_vals[CALC_MAX_CALL_ARGS];
            for (size_t i = 0; i < ast->as.call.arg_count; ++i) {
                if (!eval_node_float(ast->as.call.args[i], vars, config, &arg_vals[i], err)) {
                    return false;
                }
            }
            return eval_call_float(ast, arg_vals, out_f, err);
        }

        case AST_ASSIGN: {
            double expr_val = 0.0;
            if (!eval_node_float(ast->as.assign.expr, vars, config, &expr_val, err)) {
                return false;
            }
            CalcValue cv = calc_value_from_float(expr_val);
            if (!calc_vars_set(vars, ast->as.assign.name, &cv, err, ast->span)) {
                return false;
            }
            *out_f = expr_val;
            return true;
        }
    }

    return false;
}

static bool eval_node_exact(const CalcAstNode *ast, CalcVarTable *vars, const CalcEvalConfig *config, CalcFraction *out_frac, CalcError *err) {
    if (ast == NULL) return false;
    (void)config;

    switch (ast->kind) {
        case AST_NUMBER: {
            if (ast->as.number.val.kind == CALC_VAL_EXACT) {
                *out_frac = ast->as.number.val.v.exact;
                return true;
            }
            calc_error_set(err, CALC_ERR_DOMAIN, ast->span, "Floating point literal not supported in exact mode; use ':mode float'");
            return false;
        }

        case AST_IDENT: {
            CalcValue val;
            if (!calc_vars_get(vars, ast->as.ident.name, &val)) {
                calc_error_set(err, CALC_ERR_UNKNOWN_IDENT, ast->span, "Unknown variable '%s'", ast->as.ident.name);
                return false;
            }
            if (val.kind == CALC_VAL_EXACT) {
                *out_frac = val.v.exact;
                return true;
            }
            calc_error_set(err, CALC_ERR_DOMAIN, ast->span, "Variable '%s' contains float; switch to ':mode float'", ast->as.ident.name);
            return false;
        }

        case AST_UNARY: {
            CalcFraction operand = {0, 1};
            if (!eval_node_exact(ast->as.unary.operand, vars, config, &operand, err)) {
                return false;
            }
            if (ast->as.unary.op == OP_UNARY_PLUS) {
                *out_frac = operand;
                return true;
            }
            return calc_fraction_neg(operand, out_frac, err, ast->span);
        }

        case AST_POSTFIX: {
            CalcFraction operand = {0, 1};
            if (!eval_node_exact(ast->as.postfix.operand, vars, config, &operand, err)) {
                return false;
            }
            if (ast->as.postfix.op == OP_POSTFIX_FACTORIAL) {
                return calc_fraction_factorial(operand, out_frac, err, ast->span);
            }
            return false;
        }

        case AST_BINARY: {
            CalcFraction left = {0, 1}, right = {0, 1};
            if (!eval_node_exact(ast->as.binary.left, vars, config, &left, err)) {
                return false;
            }
            if (!eval_node_exact(ast->as.binary.right, vars, config, &right, err)) {
                return false;
            }

            switch (ast->as.binary.op) {
                case OP_ADD:
                    return calc_fraction_add(left, right, out_frac, err, ast->span);
                case OP_SUB:
                    return calc_fraction_sub(left, right, out_frac, err, ast->span);
                case OP_MUL:
                    return calc_fraction_mul(left, right, out_frac, err, ast->span);
                case OP_DIV:
                    return calc_fraction_div(left, right, out_frac, err, ast->span);
                case OP_MOD:
                    return calc_fraction_mod(left, right, out_frac, err, ast->span);
                case OP_POW:
                    if (right.den != 1) {
                        calc_error_set(err, CALC_ERR_NOT_AN_INTEGER, ast->span, "Exact power '^' requires integer exponent; use ':mode float'");
                        return false;
                    }
                    return calc_fraction_pow(left, right.num, out_frac, err, ast->span);
            }
            return false;
        }

        case AST_CALL: {
            CalcFraction arg_fracs[CALC_MAX_CALL_ARGS];
            for (size_t i = 0; i < ast->as.call.arg_count; ++i) {
                if (!eval_node_exact(ast->as.call.args[i], vars, config, &arg_fracs[i], err)) {
                    return false;
                }
            }
            return eval_call_exact(ast, arg_fracs, out_frac, err);
        }

        case AST_ASSIGN: {
            CalcFraction expr_frac = {0, 1};
            if (!eval_node_exact(ast->as.assign.expr, vars, config, &expr_frac, err)) {
                return false;
            }
            CalcValue cv = calc_value_from_exact(expr_frac);
            if (!calc_vars_set(vars, ast->as.assign.name, &cv, err, ast->span)) {
                return false;
            }
            *out_frac = expr_frac;
            return true;
        }
    }

    return false;
}

bool calc_eval_ast(const CalcAstNode *ast,
                   CalcVarTable *vars,
                   const CalcEvalConfig *config,
                   CalcValue *out_val,
                   CalcError *err) {
    if (ast == NULL || vars == NULL || out_val == NULL) {
        return false;
    }

    CalcEvalConfig cfg = (config != NULL) ? *config : calc_eval_config_default();

    if (cfg.mode == CALC_MODE_FLOAT) {
        double fval = 0.0;
        if (!eval_node_float(ast, vars, &cfg, &fval, err)) {
            return false;
        }
        *out_val = calc_value_from_float(fval);
        return true;
    }

    CalcFraction frac = {0, 1};
    if (!eval_node_exact(ast, vars, &cfg, &frac, err)) {
        return false;
    }
    *out_val = calc_value_from_exact(frac);
    return true;
}

bool calc_eval_string(const char *expr,
                      size_t len,
                      CalcVarTable *vars,
                      const CalcEvalConfig *config,
                      CalcArena *arena,
                      CalcValue *out_val,
                      CalcError *err) {
    if (expr == NULL || vars == NULL || arena == NULL || out_val == NULL) {
        return false;
    }

    calc_error_init(err);

    CalcAstNode *ast = NULL;
    if (!calc_parse(expr, len, arena, &ast, err)) {
        return false;
    }

    if (!calc_eval_ast(ast, vars, config, out_val, err)) {
        return false;
    }

    calc_vars_set_ans(vars, out_val);
    return true;
}
