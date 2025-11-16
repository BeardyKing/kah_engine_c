#ifndef ASSERT_H
#define ASSERT_H

//===INCLUDES==================================================================
#include <kah_core/defines.h>

#include <stdbool.h>
//=============================================================================

//===IMPL======================================================================
bool _core_assert_impl(bool condition, const char* file, const char* function, int line);
bool _core_assert_msg_impl(bool condition, const char* file, const char* function, int line, const char* fmt, ...);
bool _core_not_implemented_impl(const char* file, const char* function, int line);
bool _core_sanity_impl(const char* file, const char* function, int line);
bool _core_sanity_msg_impl(const char* file, const char* function, int line, const char* fmt, ...);
//=============================================================================

//===API=======================================================================
#if CHECK_FEATURE(FEATURE_ASSERTS)
#define core_assert(condition)          if (!_core_assert_impl((condition), __FILE__, __FUNCTION__, __LINE__)) core_debug_break()
#define core_assert_msg(condition, ...) if (!_core_assert_msg_impl((condition), __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)) core_debug_break()
#define core_not_implemented()          if (!_core_not_implemented_impl(__FILE__, __FUNCTION__, __LINE__)) core_debug_break()
#define core_sanity()                   if (!_core_not_implemented_impl(__FILE__, __FUNCTION__, __LINE__)) core_debug_break()
#define core_sanity_msg(...)            if (!_core_not_implemented_impl(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__))) core_debug_break()
#else
#define core_assert(condition)          ((void)(condition))
#define core_assert_msg(condition, ...) ((void)(condition))
#define core_not_implemented()          (void)0
#define core_sanity()                   (void)0
#define core_sanity_msg(...)            (void)0
#endif

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
