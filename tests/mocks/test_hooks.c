/*
 * Test hooks for host tests. Provide weak/default implementations for
 * test-level hooks that may be supplied by archived compatibility layers
 * or by specific test targets. Defining them weak here ensures tests
 * that don't link the archive still build, while archived definitions
 * (also weak) will override behavior when present.
 */

#include <stdint.h>

#if defined(__GNUC__) || defined(__clang__)
__attribute__((weak)) void Test_ResetApplication(void) { /* noop */
}
#elif defined(_MSC_VER)
__declspec(selectany) void Test_ResetApplication(void) { /* noop */
}
#else
void Test_ResetApplication(void) { /* noop */
}
#endif
