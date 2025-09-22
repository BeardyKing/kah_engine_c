//===INCLUDES==================================================================
#include <kah_core/assert.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
//=============================================================================

//===INTERNAL_FUNCTIONS=======================================================
static void internal_core_print_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

static void internal_core_log_current_position(const char* file, const char* function, int line) {
    internal_core_print_error("Assertion triggered \n\t file: \t %s \n\t func: \t %s \n\t line: \t %d \n", file, function, line);
}
//=============================================================================

//===API=======================================================================
bool _core_assert_impl(bool condition, const char* file, const char* function, int line) {
    if (!condition) {
        internal_core_log_current_position(file, function, line);
        return false;
    }
    return true;
}

bool _core_assert_msg_impl(bool condition, const char* file, const char* function, int line, const char* fmt, ...) {
    if (!condition) {
        internal_core_log_current_position(file, function, line);
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        return false;
    }
    return true;
}

bool _core_not_implemented_impl(const char* file, const char* function, int line) {
    internal_core_log_current_position(file, function, line);
    internal_core_print_error("Hit unimplemented code o_o\n");
    return false;
}

bool _core_sanity_impl(const char* file, const char* function, int line){
    internal_core_log_current_position(file, function, line);
    internal_core_print_error("Failed sanity check\n");
    return false;
}
bool _core_sanity_msg_impl(const char* file, const char* function, int line, const char* fmt, ...){
    internal_core_log_current_position(file, function, line);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    return false;

}
//=============================================================================