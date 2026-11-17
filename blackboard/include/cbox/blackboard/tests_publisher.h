#ifndef CBOX_TESTS_PUBLISHER_H
#define CBOX_TESTS_PUBLISHER_H
#include <stddef.h>

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

#endif // CBOX_TESTS_PUBLISHER_H
