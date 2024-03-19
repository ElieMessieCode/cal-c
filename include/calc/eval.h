#ifndef CALC_EVAL_H
#define CALC_EVAL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "calc/ast.h"
#include "calc/vars.h"
#include "calc/value.h"
#include "calc/error.h"
#include "calc/arena.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CALC_DEFAULT_PRECISION 15
#define CALC_MIN_PRECISION 1
#define CALC_MAX_PRECISION 17

typedef enum CalcEvalMode {
    CALC_MODE_FLOAT = 0,
    CALC_MODE_EXACT
} CalcEvalMode;

typedef struct CalcEvalConfig {
    CalcEvalMode mode;
    int          precision;
} CalcEvalConfig;

CalcEvalConfig calc_eval_config_default(void);

bool calc_eval_ast(const CalcAstNode     *ast,
                   CalcVarTable          *vars,
                   const CalcEvalConfig  *config,
                   CalcValue             *out_val,
                   CalcError             *err);

bool calc_eval_string(const char            *expr,
                      size_t                 len,
                      CalcVarTable          *vars,
                      const CalcEvalConfig  *config,
                      CalcArena             *arena,
                      CalcValue             *out_val,
                      CalcError             *err);

#ifdef __cplusplus
}
#endif

#endif
