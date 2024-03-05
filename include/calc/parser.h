#ifndef CALC_PARSER_H
#define CALC_PARSER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "calc/arena.h"
#include "calc/ast.h"
#include "calc/error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CALC_MAX_INPUT_LEN 4096

bool calc_parse(const char *src, size_t len, CalcArena *arena, CalcAstNode **out_ast, CalcError *err);

#ifdef __cplusplus
}
#endif

#endif
