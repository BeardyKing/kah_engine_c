//===INCLUDES==================================================================
#include <kah_core/assert.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
//=============================================================================

//===INTERNAL_FUNCTIONS=======================================================
__attribute__((format(printf, 1, 2)))
static void internal_core_print_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    fflush(stdout);
}

static void internal_core_log_current_position(const char* file, const char* function, int line) {
    internal_core_print_error("Assertion triggered \n\t file: \t %s \n\t func: \t %s \n\t line: \t %d \n", file, function, line);
}
//=============================================================================

//===API=======================================================================
bool internal_core_assert_impl(bool condition, const char* file, const char* function, int line) {
    if (!condition) {
        internal_core_log_current_position(file, function, line);
        return false;
    }
    return true;
}

__attribute__((format(printf, 5, 6)))
bool internal_core_assert_msg_impl(bool condition, const char* file, const char* function, int line, const char* fmt, ...) {
    if (!condition) {
        internal_core_log_current_position(file, function, line);
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        fflush(stdout);
        fflush(stderr);
        return false;
    }
    return true;
}

bool internal_core_not_implemented_impl(const char* file, const char* function, int line) {
    internal_core_log_current_position(file, function, line);
    internal_core_print_error("Hit unimplemented code o_o\n");
    return false;
}

bool internal_core_sanity_impl(const char* file, const char* function, int line){
    internal_core_log_current_position(file, function, line);
    internal_core_print_error("Failed sanity check\n");
    return false;
}

__attribute__((format(printf, 4, 5)))
bool internal_core_sanity_msg_impl(const char* file, const char* function, int line, const char* fmt, ...){
    internal_core_log_current_position(file, function, line);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    fflush(stdout);
    return false;

}
//=============================================================================
