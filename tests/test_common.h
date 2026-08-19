#ifndef FRACTUS_TEST_COMMON_H
#define FRACTUS_TEST_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static int g_tests_run = 0;
static int g_tests_failed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("  [FAIL] %s:%d: %s (%s)\n", __FILE__, __LINE__, msg, #cond); \
            g_tests_failed++; \
            return 0; \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL_INT(expected, actual, msg) \
    do { \
        int _exp = (int)(expected); \
        int _act = (int)(actual); \
        if (_exp != _act) { \
            printf("  [FAIL] %s:%d: %s (expected %d, got %d)\n", __FILE__, __LINE__, msg, _exp, _act); \
            g_tests_failed++; \
            return 0; \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL_DOUBLE(expected, actual, delta, msg) \
    do { \
        double _exp = (double)(expected); \
        double _act = (double)(actual); \
        if (fabs(_exp - _act) > (delta)) { \
            printf("  [FAIL] %s:%d: %s (expected %.6f, got %.6f)\n", __FILE__, __LINE__, msg, _exp, _act); \
            g_tests_failed++; \
            return 0; \
        } \
    } while (0)

#define TEST_RUN(test_func) \
    do { \
        g_tests_run++; \
        printf("[RUN ] %s\n", #test_func); \
        if (test_func()) { \
            printf("[PASS] %s\n", #test_func); \
        } \
    } while (0)

#define TEST_REPORT() \
    do { \
        printf("\n========================================\n"); \
        printf("Tests run: %d | Passed: %d | Failed: %d\n", \
               g_tests_run, g_tests_run - g_tests_failed, g_tests_failed); \
        printf("========================================\n"); \
        return (g_tests_failed == 0) ? 0 : 1; \
    } while (0)

#endif
