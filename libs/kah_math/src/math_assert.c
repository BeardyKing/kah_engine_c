//===INCLUDES==================================================================
#include <kah_math/math_assert.h>

#include <stdarg.h>
#include <stdio.h>
//=============================================================================

//===INTERNAL_FUNCTIONS=======================================================
__attribute__((format(printf, 1, 2)))
static void internal_math_print_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    fflush(stdout);
}

static void internal_math_log_current_position(const char* file, const char* function, int line) {
    internal_math_print_error("Assertion triggered \n\t file: \t %s \n\t func: \t %s \n\t line: \t %d \n", file, function, line);
}
//=============================================================================

//===API=======================================================================
bool internal_math_assert_impl(bool condition, const char* file, const char* function, int line) {
    if (!condition) {
        internal_math_log_current_position(file, function, line);
        return false;
    }
    return true;
}
//=============================================================================
