#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "cbox/essentials/result.h"

#ifdef NDEBUG
#error "Result unit tests require assertions; compile without -DNDEBUG."
#endif

/*
 * Teaching tests for our first real Essentials module.
 * These are deliberately committed before result.c so the learner can
 * implement the contract and then make the tests pass (RED -> GREEN).
 * A PASSED line is printed only after every assertion in that test succeeds.
 */
static void test_ok(void)
{
    cbox_result result = cbox_result_ok();

    assert(result.status == CBOX_RESULT_OK);
    assert(result.error_code == 0);
    assert(result.message == NULL);

    printf("%-42s PASSED\n", __func__);
}

static void test_failure_with_description(void)
{
    cbox_result result = cbox_result_fail(404, "File not found");

    assert(result.status == CBOX_RESULT_FAIL);
    assert(result.error_code == 404);
    assert(result.message != NULL);
    assert(strcmp(result.message, "File not found") == 0);

    printf("%-42s PASSED\n", __func__);
}

static void test_failure_without_description(void)
{
    cbox_result result = cbox_result_fail(0, NULL);

    assert(result.status == CBOX_RESULT_FAIL);
    assert(result.error_code == 0);
    assert(result.message == NULL);

    printf("%-42s PASSED\n", __func__);
}

static void test_result_returned_by_value(void)
{
    cbox_result result = cbox_result_fail(7, "Permission denied");
    cbox_result copy = result;

    assert(copy.status == CBOX_RESULT_FAIL);
    assert(copy.error_code == 7);
    assert(copy.message == result.message);

    printf("%-42s PASSED\n", __func__);
}

int main(void)
{
    test_ok();
    test_failure_with_description();
    test_failure_without_description();
    test_result_returned_by_value();

    puts("\nCBox Essentials Result: all tests PASSED.");
    return 0;
}
