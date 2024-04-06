#ifndef CALC_TEST_H
#define CALC_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;
static const char *g_current_test_name = "";

#define TEST_BEGIN(name) \
    static void name(void) { \
        g_current_test_name = #name; \
        g_tests_run++; \
        bool _test_ok = true;

#define TEST_END() \
        if (_test_ok) { \
            g_tests_passed++; \
            printf("  [PASS] %s\n", g_current_test_name); \
        } else { \
            g_tests_failed++; \
            printf("  [FAIL] %s\n", g_current_test_name); \
        } \
    }

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        printf("    ASSERTION FAILED: %s (at %s:%d)\n", #cond, __FILE__, __LINE__); \
        _test_ok = false; \
        return; \
    } \
} while (0)

#define ASSERT_FALSE(cond) do { \
    if (cond) { \
        printf("    ASSERTION FAILED (expected false): %s (at %s:%d)\n", #cond, __FILE__, __LINE__); \
        _test_ok = false; \
        return; \
    } \
} while (0)

#define ASSERT_EQ_INT(a, b) do { \
    int64_t _a = (int64_t)(a); \
    int64_t _b = (int64_t)(b); \
    if (_a != _b) { \
        printf("    ASSERTION FAILED: %s == %s (%" PRId64 " != %" PRId64 ") (at %s:%d)\n", #a, #b, _a, _b, __FILE__, __LINE__); \
        _test_ok = false; \
        return; \
    } \
} while (0)

#define ASSERT_EQ_STR(a, b) do { \
    const char *_sa = (a); \
    const char *_sb = (b); \
    if (_sa == NULL || _sb == NULL || strcmp(_sa, _sb) != 0) { \
        printf("    ASSERTION FAILED: \"%s\" == \"%s\" (at %s:%d)\n", _sa ? _sa : "NULL", _sb ? _sb : "NULL", __FILE__, __LINE__); \
        _test_ok = false; \
        return; \
    } \
} while (0)

#define ASSERT_NEAR(a, b, eps) do { \
    double _da = (double)(a); \
    double _db = (double)(b); \
    double _diff = fabs(_da - _db); \
    if (_diff > (eps)) { \
        printf("    ASSERTION FAILED: |%s - %s| <= %g (|%g - %g| = %g) (at %s:%d)\n", #a, #b, (double)(eps), _da, _db, _diff, __FILE__, __LINE__); \
        _test_ok = false; \
        return; \
    } \
} while (0)

#define RUN_TEST(test_fn) do { \
    test_fn(); \
} while (0)

#define TEST_REPORT() do { \
    printf("\n--- Test Results: %d run, %d passed, %d failed ---\n\n", g_tests_run, g_tests_passed, g_tests_failed); \
    if (g_tests_failed > 0) { \
        return 1; \
    } \
    return 0; \
} while (0)

#endif
