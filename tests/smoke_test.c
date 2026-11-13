#include <limits.h>
#include <stdio.h>

/*
 * This tests only that the project can compile and run a C17 program.
 * It is NOT a substitute for unit tests of the future CBox modules.
 */
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201710L
#error "CBox requires a C17-compatible compilation mode."
#endif

_Static_assert(CHAR_BIT >= 8, "C requires at least eight bits per byte.");

int main(void)
{
    puts("CBox toolchain smoke test passed (library modules not implemented yet).");
    return 0;
}
