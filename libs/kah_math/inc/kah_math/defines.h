#ifndef MATH_DEFINES_H
#define MATH_DEFINES_H

//===INCLUDES==================================================================
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
//=============================================================================

//===COMPILER==================================================================
#if defined(_MSC_VER)
    #define MATH_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define MATH_FORCE_INLINE inline __attribute__((always_inline))
#else
    #define MATH_FORCE_INLINE inline
#endif
//=============================================================================

//===UTILS=====================================================================
#define KAH_PI_FLOAT 3.1415927f
#define KAH_PI_DOUBLE 3.141592653589793
//=============================================================================

#endif //MATH_DEFINES_H
