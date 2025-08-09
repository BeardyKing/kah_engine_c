#ifndef ASSERT_H
#define ASSERT_H

//===INCLUES======================================================================
#include <kah_core/defines.h>

#include <stdbool.h>
//=============================================================================

//===IMPL======================================================================
void _core_assert_impl(bool condition, const char* file, const char* function, int line);
void _core_assert_msg_impl(bool condition, const char* file, const char* function, int line, const char* fmt, ...);
void _core_not_implemented_impl(const char* file, const char* function, int line);
//=============================================================================

//===API=======================================================================
#define core_assert(condition)          _core_assert_impl((condition), __FILE__, __FUNCTION__, __LINE__)
#define core_assert_msg(condition, ...) _core_assert_msg_impl((condition), __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define core_not_implemented()          _core_not_implemented_impl(__FILE__, __FUNCTION__, __LINE__)

#if defined(__linux__) || defined(__APPLE__)
#include <csignal>
#define core_debug_break() raise(SIGTRAP)
#elif defined(_WIN32)
#define core_debug_break() __debugbreak()
#else
#define core_debug_break() // not implemented
#endif

//=============================================================================

#endif //ASSERT_H
