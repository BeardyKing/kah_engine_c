//===INCLUES======================================================================
#include <kah_core/assert.h>

#include <stdarg.h>
#include <stdio.h>
//================================================================================

//===INTERNAL_FUNCTIONS===========================================================
static void internal_core_print_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

static void internal_core_log_current_position(const char* file, const char* function, int line) {
    internal_core_print_error("Assertion triggered \n\t file: \t %s \n\t func: \t %s \n\t line: \t %d \n", file, function, line);
}
//================================================================================

//===API==========================================================================
void _core_assert_impl(bool condition, const char* file, const char* function, int line) {
    if (!condition) {
        internal_core_log_current_position(file, function, line);
        core_debug_break();
    }
}

void _core_assert_msg_impl(bool condition, const char* file, const char* function, int line, const char* fmt, ...) {
    if (!condition) {
        internal_core_log_current_position(file, function, line);
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        core_debug_break();
    }
}

void _core_not_implemented_impl(const char* file, const char* function, int line) {
    internal_core_log_current_position(file, function, line);
    internal_core_print_error("Hit unimplemented code o_o\n");
    core_debug_break();
}
//================================================================================