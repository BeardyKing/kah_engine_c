#ifndef MATH_DEFINES_H
#define MATH_DEFINES_H
//===COMPILER==================================================================
#if defined(_MSC_VER)
    #define MATH_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define MATH_FORCE_INLINE inline __attribute__((always_inline))
#else
    #define MATH_FORCE_INLINE inline
#endif
//=============================================================================
#endif //MATH_DEFINES_H
