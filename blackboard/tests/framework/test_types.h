#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#define MAX_ASSERTIONS 1024
#define MAX_TESTS_PER_SCOPE 1024
#define MAX_SCOPES 1024
#define MAX_TEST_NAME_LENGTH 128
#define MAX_SCOPE_NAME_LENGTH 128
#define MAX_FAIL_MESSAGE_LENGTH 512

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Structure representing the result of an assertion within a test
 * @param assert_name The name of the assertion
 * @param order The order of the assertion within the test
 * @param fail_message The failure message if the assertion failed
 * @param passed Indicates whether the assertion passed or failed
 * @note This structure is used to store the result of an individual assertion within a test.
 */
typedef struct assert_result
{
    const char assert_name[MAX_TEST_NAME_LENGTH];
    unsigned int *order;
    const char fail_message[MAX_FAIL_MESSAGE_LENGTH];
    bool passed;
} assert_result;

/**
 * @brief Structure representing the statistics of a test
 * @param test_name The name of the test
 * @param order The order of the test
 * @param local_assert_results Array of local assertion results for the test
 * @param local_asserts_passed Number of local assertions passed
 * @param local_asserts_failed Number of local assertions failed
 * @note This structure is used to store the statistics of an individual test.
 */
typedef struct test_stats
{
    const char test_name[MAX_TEST_NAME_LENGTH];
    unsigned int *order;
    assert_result local_assert_results[MAX_ASSERTIONS];
    unsigned int local_asserts_passed;
    unsigned int local_asserts_failed;
} test_stats;

/**
 * @brief Structure representing the statistics of a scope
 * @param scope_name The name of the scope
 * @param tests Array of test statistics within the scope
 * @param test_count Number of tests within the scope
 * @note This structure is used to store the statistics of an individual scope.
 */
typedef struct scope_stats
{
    const char scope_name[MAX_SCOPE_NAME_LENGTH];
    test_stats tests[MAX_TESTS_PER_SCOPE];
    size_t test_count;
} scope_stats;

/**
 * @brief Structure representing the statistics of a test suite
 * @param scopes Array of scope statistics within the suite
 * @param scope_count Number of scopes within the suite
 * @param tests_passed Number of tests passed in the suite
 * @param tests_failed Number of tests failed in the suite
 * @param asserts_passed Number of assertions passed in the suite
 * @param asserts_failed Number of assertions failed in the suite
 * @param total_tests Total number of tests in the suite
 * @param total_asserts Total number of assertions in the suite
 * @param total_scopes Total number of scopes in the suite
 * @param all_tests_passed Indicates whether all tests in the suite passed
 * @note This structure is used to store the overall statistics of a test suite.
 */
typedef struct suite_stats
{
    scope_stats scopes[MAX_SCOPES];
    size_t scope_count;
    unsigned int tests_passed;
    unsigned int tests_failed;
    unsigned int asserts_passed;
    unsigned int asserts_failed;
    unsigned int total_tests;
    unsigned int total_asserts;
    unsigned int total_scopes;
    bool all_tests_passed;
} suite_stats;

/**
 * @brief Structure representing an individual assert
 * @param assert_name The name of the assert
 * @param run_assert Function pointer to run the assert
 */
typedef struct assert
{
    char *assert_name;
    assert_result (*run_assert)(void);
} assert;

/**
 * @brief Structure representing an individual test
 * @param test_name The name of the test
 * @param run_tests Function pointer to run the test
 */
typedef struct test
{
    char *test_name;
    void (*run_tests)(void);
} test;

/**
 * @brief Structure representing an individual scope
 * @param scope_name The name of the scope
 * @param run_tests Function pointer to run the tests within the scope
 */
typedef struct scope
{
    char *scope_name;
    void (*run_tests)(void);
} scope;

/**
 * @brief Structure representing an individual test suite
 * @param suite_name The name of the test suite
 * @param run_tests Function pointer to run the tests within the suite
 */
typedef struct suite
{
    char *suite_name;
    void (*run_tests)(void);

} suite;

#endif // TEST_TYPES_H
