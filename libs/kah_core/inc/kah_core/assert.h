#ifndef ASSERT_H
#define ASSERT_H

#include <stdio.h>
#include <kah_core/defines.h>

#if defined(__linux__) || defined(__APPLE__)
#include <csignal>
#define core_debug_break() raise(SIGTRAP)
#elif defined(_WIN32)
#define core_debug_break() __debugbreak()
#else
#define core_debug_break() // not implemented
#endif

#define core_print_error(...) printf(__VA_ARGS__)
#define core_log_current_position() core_print_error("Assertion triggered \n\t file: \t %s \n\t func: \t %s \n\t line: \t %d \n", __FILE__, __FUNCTION__, __LINE__)

#define core_assert(condition)              \
{                                           \
    if(!(condition)){                       \
        core_log_current_position();        \
        core_debug_break();                 \
    }                                       \
}

#define core_assert_msg(condition, ...)     \
{                                           \
    if(!(condition)){                       \
        core_log_current_position();        \
        core_print_error(__VA_ARGS__);      \
        core_debug_break();                 \
    }                                       \
}

#define core_not_implemented()                      \
{                                                   \
    core_log_current_position();                    \
    core_print_error("Hit unimplemented code o_o"); \
    core_debug_break();                             \
}

#endif //ASSERT_H
