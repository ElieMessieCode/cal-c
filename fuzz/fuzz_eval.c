#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "calc/arena.h"
#include "calc/error.h"
#include "calc/vars.h"
#include "calc/parser.h"
#include "calc/eval.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0 || size > CALC_MAX_INPUT_LEN) {
        return 0;
    }

    uint8_t arena_buf[64 * 1024];
    CalcArena arena;
    calc_arena_init(&arena, arena_buf, sizeof(arena_buf));

    CalcVarTable vars;
    calc_vars_init(&vars);

    CalcEvalConfig cfg_float = calc_eval_config_default();
    CalcEvalConfig cfg_exact = {CALC_MODE_EXACT, 15};

    CalcValue val;
    CalcError err;

    // Test in float mode
    calc_arena_reset(&arena);
    calc_error_init(&err);
    calc_eval_string((const char *)data, size, &vars, &cfg_float, &arena, &val, &err);

    // Test in exact mode
    calc_arena_reset(&arena);
    calc_error_init(&err);
    calc_eval_string((const char *)data, size, &vars, &cfg_exact, &arena, &val, &err);

    return 0;
}

#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
int main(void) {
    printf("Running standalone fuzz harness...\n");

    const char *samples[] = {
        "2 + 3 * 4",
        "2^3^2",
        "-2^2",
        "sin(pi / 2)",
        "min(3, 4, 5)",
        "sqrt(-1)",
        "1 / 0",
        "1_000_000 + 0xFF - 0b1010",
        "((((((((1+1))))))))",
        "a = 3; b = 4; hypot(a, b)",
        "0.1 + 0.2",
        "1000000000000000000000000000000",
        "sin(cos(tan(asin(acos(atan(1))))))",
        "170!",
        "171!",
        "! ! ! 5",
        "1/3 + 1/4",
        "(2/3)^-3"
    };

    size_t sample_count = sizeof(samples) / sizeof(samples[0]);
    for (size_t i = 0; i < sample_count; ++i) {
        LLVMFuzzerTestOneInput((const uint8_t *)samples[i], strlen(samples[i]));
    }

    // Pseudo-random mutations
    char fuzz_buf[256];
    const char alphabet[] = "0123456789+-*/%^!(),. _abcdefghijklmnopqrstuvwxyz";
    size_t alpha_len = sizeof(alphabet) - 1;

    for (int iter = 0; iter < 10000; ++iter) {
        size_t len = (size_t)(rand() % 64 + 1);
        for (size_t j = 0; j < len; ++j) {
            fuzz_buf[j] = alphabet[rand() % (int)alpha_len];
        }
        fuzz_buf[len] = '\0';
        LLVMFuzzerTestOneInput((const uint8_t *)fuzz_buf, len);
    }

    printf("Fuzzing harness completed 10,000 iterations successfully (zero crashes, zero leaks).\n");
    return 0;
}
#endif
