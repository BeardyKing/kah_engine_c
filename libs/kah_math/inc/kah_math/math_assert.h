#ifndef MATH_ASSERT_H
#define MATH_ASSERT_H

//===INCLUDES==================================================================
#include "stdbool.h"
//=============================================================================

//===IMPL======================================================================
bool internal_math_assert_impl(bool condition, const char* file, const char* function, int line);
//=============================================================================
//===API=======================================================================
#define math_assert(condition) if (!internal_math_assert_impl((condition), __FILE__, __FUNCTION__, __LINE__)) math_debug_break()

#if defined(__linux__) || defined(__APPLE__)
#include <signal.h>
#define math_debug_break() raise(SIGTRAP)
#elif defined(_WIN32)
#define math_debug_break() __debugbreak()
#else
#define math_debug_break() // not implemented
#endif
//=============================================================================

#endif //MATH_ASSERT_H
