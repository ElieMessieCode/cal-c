#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#if defined(_WIN32)
#include <io.h>
#define isatty_compat(fd) _isatty(fd)
#define fileno_compat(f)  _fileno(f)
#else
#include <unistd.h>
#define isatty_compat(fd) isatty(fd)
#define fileno_compat(f)  fileno(f)
#endif

#include "calc/arena.h"
#include "calc/error.h"
#include "calc/value.h"
#include "calc/vars.h"
#include "calc/parser.h"
#include "calc/eval.h"

#define CALC_VERSION "1.0.0"
#define ARENA_CAPACITY (64 * 1024)

static void print_usage(const char *prog) {
    printf("Usage:\n");
    printf("  %s [options] [expression]\n\n", prog);
    printf("Options:\n");
    printf("  -e, --eval <expr>        Evaluate expression and exit\n");
    printf("  -m, --mode <float|exact> Set evaluation mode (default: float)\n");
    printf("  -p, --precision <1-17>   Set display precision (default: 15)\n");
    printf("  -v, --version            Display version information\n");
    printf("  -h, --help               Display this help message\n\n");
    printf("Interactive Commands:\n");
    printf("  :help                    Show interactive help\n");
    printf("  :vars                    List active variables and constants\n");
    printf("  :ast <expr>              Print AST representation of expression\n");
    printf("  :mode [float|exact]      Get or set evaluation mode\n");
    printf("  :precision [1-17]        Get or set floating-point precision\n");
    printf("  :quit, :q                Exit REPL\n");
}

static void print_repl_help(void) {
    printf("\nCommands:\n");
    printf("  :help               Show this help text\n");
    printf("  :vars               Display variable table\n");
    printf("  :ast <expr>         Show AST structure of expression\n");
    printf("  :mode [float|exact] View or switch calculation mode\n");
    printf("  :precision [1-17]   View or set decimal precision\n");
    printf("  :quit, :q           Exit the calculator\n\n");
    printf("Operators:\n");
    printf("  +, -, *, /, %%, ^, !\n\n");
    printf("Built-in Functions:\n");
    printf("  sin, cos, tan, asin, acos, atan, sqrt, cbrt, exp, ln, log10\n");
    printf("  abs, floor, ceil, round, min, max, pow, hypot\n\n");
}

static void print_vars(const CalcVarTable *vars, int precision) {
    size_t count = calc_vars_count(vars);
    printf("\n  Symbol        Value               Type\n");
    printf("  ------------  ------------------  ---------\n");

    char val_buf[128];
    for (size_t i = 0; i < count; ++i) {
        const CalcVarEntry *entry = NULL;
        if (calc_vars_get_entry(vars, i, &entry)) {
            calc_value_format(&entry->value, precision, val_buf, sizeof(val_buf));
            printf("  %-12s  %-18s  %s\n",
                   entry->name,
                   val_buf,
                   entry->is_const ? "constant" : "variable");
        }
    }
    printf("\n");
}

static int get_exit_code_for_error(CalcErrCode code) {
    if (code == CALC_ERR_SYNTAX || code == CALC_ERR_TOO_LONG || code == CALC_ERR_TOO_DEEP) {
        return 2;
    }
    return 1;
}

static int run_expression(const char *expr, CalcVarTable *vars, const CalcEvalConfig *cfg, CalcArena *arena, bool print_caret) {
    calc_arena_reset(arena);
    CalcError err;
    calc_error_init(&err);

    CalcValue val;
    size_t len = strlen(expr);

    if (!calc_eval_string(expr, len, vars, cfg, arena, &val, &err)) {
        if (print_caret) {
            char caret_buf[512];
            calc_error_format_caret(&err, expr, caret_buf, sizeof(caret_buf));
            fprintf(stderr, "%s\n", caret_buf);
        } else {
            fprintf(stderr, "error: %s\n", err.msg[0] != '\0' ? err.msg : calc_errcode_str(err.code));
        }
        return get_exit_code_for_error(err.code);
    }

    char out_buf[128];
    calc_value_format(&val, cfg->precision, out_buf, sizeof(out_buf));
    printf("%s\n", out_buf);
    return 0;
}

static void run_ast_command(const char *expr, CalcArena *arena) {
    calc_arena_reset(arena);
    CalcError err;
    calc_error_init(&err);

    CalcAstNode *ast = NULL;
    size_t len = strlen(expr);
    if (!calc_parse(expr, len, arena, &ast, &err)) {
        char caret_buf[512];
        calc_error_format_caret(&err, expr, caret_buf, sizeof(caret_buf));
        fprintf(stderr, "%s\n", caret_buf);
        return;
    }

    char buf[1024];
    calc_ast_format(ast, buf, sizeof(buf));
    printf("AST: %s\n", buf);
}

int main(int argc, char **argv) {
    CalcEvalConfig cfg = calc_eval_config_default();
    CalcVarTable vars;
    calc_vars_init(&vars);

    uint8_t arena_buffer[ARENA_CAPACITY];
    CalcArena arena;
    calc_arena_init(&arena, arena_buffer, sizeof(arena_buffer));

    const char *eval_expr = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("cal-c %s\n", CALC_VERSION);
            return 0;
        }
        if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mode") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: %s requires an argument (float or exact)\n", argv[i]);
                return 64;
            }
            i++;
            if (strcmp(argv[i], "float") == 0) {
                cfg.mode = CALC_MODE_FLOAT;
            } else if (strcmp(argv[i], "exact") == 0) {
                cfg.mode = CALC_MODE_EXACT;
            } else {
                fprintf(stderr, "error: invalid mode '%s' (expected 'float' or 'exact')\n", argv[i]);
                return 64;
            }
            continue;
        }
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--precision") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: %s requires an integer precision (1-17)\n", argv[i]);
                return 64;
            }
            i++;
            int prec = atoi(argv[i]);
            if (prec < CALC_MIN_PRECISION || prec > CALC_MAX_PRECISION) {
                fprintf(stderr, "error: precision must be between %d and %d\n", CALC_MIN_PRECISION, CALC_MAX_PRECISION);
                return 64;
            }
            cfg.precision = prec;
            continue;
        }
        if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--eval") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: %s requires an expression\n", argv[i]);
                return 64;
            }
            i++;
            eval_expr = argv[i];
            continue;
        }
        if (argv[i][0] == '-' && argv[i][1] == '-') {
            fprintf(stderr, "error: unrecognized option '%s'\n", argv[i]);
            return 64;
        }
        if (argv[i][0] == '-' && (argv[i][1] == 'h' || argv[i][1] == 'v' || argv[i][1] == 'm' || argv[i][1] == 'p' || argv[i][1] == 'e') && argv[i][2] == '\0') {
            fprintf(stderr, "error: unrecognized option '%s'\n", argv[i]);
            return 64;
        }
        if (eval_expr == NULL) {
            eval_expr = argv[i];
        } else {
            fprintf(stderr, "error: unexpected positional argument '%s'\n", argv[i]);
            return 64;
        }
    }

    if (eval_expr != NULL) {
        return run_expression(eval_expr, &vars, &cfg, &arena, true);
    }

    bool is_interactive = isatty_compat(fileno_compat(stdin)) != 0;

    if (is_interactive) {
        printf("cal-c %s\n", CALC_VERSION);
        printf("type :help for commands, :quit to exit\n\n");
    }

    char line_buf[4096];
    int last_exit_code = 0;

    while (1) {
        if (is_interactive) {
            printf("calc> ");
            fflush(stdout);
        }

        if (fgets(line_buf, sizeof(line_buf), stdin) == NULL) {
            if (is_interactive) {
                printf("\n");
            }
            break;
        }

        size_t line_len = strlen(line_buf);
        while (line_len > 0 && (line_buf[line_len - 1] == '\n' || line_buf[line_len - 1] == '\r')) {
            line_buf[--line_len] = '\0';
        }

        size_t start = 0;
        while (line_buf[start] == ' ' || line_buf[start] == '\t') {
            start++;
        }
        if (line_buf[start] == '\0') {
            continue;
        }

        char *cmd = &line_buf[start];

        if (cmd[0] == ':') {
            if (strcmp(cmd, ":quit") == 0 || strcmp(cmd, ":q") == 0) {
                break;
            }
            if (strcmp(cmd, ":help") == 0 || strcmp(cmd, ":h") == 0) {
                print_repl_help();
                continue;
            }
            if (strcmp(cmd, ":vars") == 0) {
                print_vars(&vars, cfg.precision);
                continue;
            }
            if (strncmp(cmd, ":ast", 4) == 0) {
                const char *expr_part = cmd + 4;
                while (*expr_part == ' ' || *expr_part == '\t') expr_part++;
                if (*expr_part == '\0') {
                    fprintf(stderr, "error: :ast requires an expression\n");
                } else {
                    run_ast_command(expr_part, &arena);
                }
                continue;
            }
            if (strncmp(cmd, ":mode", 5) == 0) {
                const char *arg = cmd + 5;
                while (*arg == ' ' || *arg == '\t') arg++;
                if (*arg == '\0') {
                    printf("mode: %s\n", cfg.mode == CALC_MODE_FLOAT ? "float" : "exact");
                } else if (strcmp(arg, "float") == 0) {
                    cfg.mode = CALC_MODE_FLOAT;
                    printf("mode switched to float\n");
                } else if (strcmp(arg, "exact") == 0) {
                    cfg.mode = CALC_MODE_EXACT;
                    printf("mode switched to exact\n");
                } else {
                    fprintf(stderr, "error: invalid mode '%s' (use 'float' or 'exact')\n", arg);
                }
                continue;
            }
            if (strncmp(cmd, ":precision", 10) == 0) {
                const char *arg = cmd + 10;
                while (*arg == ' ' || *arg == '\t') arg++;
                if (*arg == '\0') {
                    printf("precision: %d\n", cfg.precision);
                } else {
                    int prec = atoi(arg);
                    if (prec < CALC_MIN_PRECISION || prec > CALC_MAX_PRECISION) {
                        fprintf(stderr, "error: precision must be between %d and %d\n", CALC_MIN_PRECISION, CALC_MAX_PRECISION);
                    } else {
                        cfg.precision = prec;
                        printf("precision set to %d\n", cfg.precision);
                    }
                }
                continue;
            }
            fprintf(stderr, "error: unknown command '%s' (type :help for available commands)\n", cmd);
            continue;
        }

        int code = run_expression(cmd, &vars, &cfg, &arena, is_interactive);
        if (code != 0) {
            last_exit_code = code;
        }
    }

    return last_exit_code;
}
