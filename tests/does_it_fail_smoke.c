/* Optional consumer-side link smoke test (independent of Result exercise). */
#include <stdio.h>
#include "does_it_fail/test.h"
#include "does_it_fail/reporter_console.h"

static void test_cbox_can_link(dif_test_context *context)
{
    DIF_EXPECT_INT_EQ(context, 5, 2 + 3);
}

static void test_runner_continues(dif_test_context *context)
{
    DIF_EXPECT_INT_EQ(context, 0, 0);
    DIF_EXPECT_INT_EQ(context, 17, 17);
}

int main(void)
{
    const dif_test_case cases[] =
    {
        {"test_cbox_can_link", test_cbox_can_link},
        {"test_runner_continues", test_runner_continues}
    };
    dif_console_options console = {0};
    console.stream = stdout;
    console.color = false; /* CI-safe by default. */
    dif_summary summary = dif_run_suite("CBox / DoesItFail integration", cases,
        sizeof(cases) / sizeof(cases[0]), dif_report_console, &console);
    return (summary.setup_error || summary.failed_tests != 0) ? 1 : 0;
}
