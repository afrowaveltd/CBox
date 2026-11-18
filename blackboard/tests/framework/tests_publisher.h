#ifndef CBOX_TESTS_PUBLISHER_H
#define CBOX_TESTS_PUBLISHER_H
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Maximum number of local assertions per test
 */
const size_t MAX_LOCAL_ASSERTS = 1000ul;

/**
 * @brief Maximum number of tests per pointer
 */
const size_t MAX_TESTS_PER_POINTER = 1000ul;

/**
 * @brief Maximum number of test scope statistics
 */
const size_t MAX_TEST_SCOPE_STATS = 1000ul;

/**
 * @brief Maximum number of test suite statistics
 */ 
const size_t MAX_TEST_SUITE_STATS = 1000ul;

/**
 * @brief Enumeration for text styles used in test output
 */
enum cbox_text_style
{
    PASS,
    FAIL,
    WARN,
    INFO
};

/**
 * @brief Structure representing the result of an assertion
 * @param assert_name The name of the assertion
 * @param test_name The name of the test containing the assertion
 * @param order The order of the assertion within the test
 * @param fail_message The failure message if the assertion failed
 * @param style The style of the assertion result (PASS, FAIL, WARN, INFO)
 * @param passed Indicates whether the assertion passed or failed
 * @note This structure is used to store the result of an individual assertion within a test.
 */
typedef struct assert_result
{
    const char *assert_name;
    unsigned int *order;
    const char *fail_message;
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
    const char *test_name;
    unsigned int order;
    assert_result local_assert_results[100];
    unsigned int local_asserts_passed;
    unsigned int local_asserts_failed;
} test_stats;

typedef struct scope_stats
{
    const char *scope_name;
    test_stats *tests;
    size_t test_count;

} scope_stats;
typedef struct suite_stats
{
    scope_stats *tests;
    size_t test_count;
    unsigned int tests_passed;
    unsigned int tests_failed;
    unsigned int asserts_passed;
    unsigned int asserts_failed;
    unsigned int total_tests;
} suite_stats;





/**
 * @brief Prints 40 equal signs as a thick line separator
 *
 */
void cbox_print_thick_line(void);

/**
 * @brief Prints 40 hyphens as a thin line separator
 *
 */
void cbox_print_thin_line(void);

/**
 * @brief Starts a test with the given name and order
 * @param test_name The name of the test to start
 * @param p_order Pointer to the order of the test
 * @note This function should be called at the beginning of each test to initialize its order.
 */
void cbox_start_test(
    const char *test_name,
    unsigned int *p_order);

/**
 * @brief Prints the result of an assertion with the given message and style
 * @param message The message to print
 * @param style The style of the message (PASS, FAIL, WARN, INFO)
 */
void cbox_print_assert_result(
    const char *message,
    enum cbox_text_style style);

/**
 * @brief Prints the result of a test with the given details
 *
 * @param test_name The name of the test
 * @param style The style of the test result (PASS, FAIL, WARN, INFO)
 * @param p_order Pointer to the order of the test
 * @param p_local_asserts_passed Pointer to the number of local assertions passed
 * @param p_local_asserts_failed Pointer to the number of local assertions failed
 * @param p_tests_passed Pointer to the number of tests passed
 * @param p_tests_failed Pointer to the number of tests failed
 */
void cbox_print_test_result(
    const char *test_name,
    enum cbox_text_style style,
    unsigned int *p_order,
    unsigned int *p_local_asserts_passed,
    unsigned int *p_local_asserts_failed,
    unsigned int *p_tests_passed,
    unsigned int *p_tests_failed);

/**
 * @brief Ends the current test and updates the test statistics
 *
 * @param p_order Pointer to the order of the test
 * @param p_local_asserts_passed Pointer to the number of local assertions passed
 * @param p_local_asserts_failed Pointer to the number of local assertions failed
 * @param p_tests_passed Pointer to the number of tests passed
 * @param p_tests_failed Pointer to the number of tests failed
 */
void cbox_end_test(
    unsigned int *p_order,
    unsigned int *p_local_asserts_passed,
    unsigned int *p_local_asserts_failed,
    unsigned int *p_tests_passed,
    unsigned int *p_tests_failed);

/**
 * @brief Prints the errors for the failed tests
 *
 * @param count The number of failed tests
 * @param test_names Array of test names
 * @param error_messages Array of error messages corresponding to the failed tests
 */
void cbox_print_errors(
    size_t count,
    const char *const test_names[],
    const char *const error_messages[]);

#endif CBOX_TESTS_PUBLISHER_H
