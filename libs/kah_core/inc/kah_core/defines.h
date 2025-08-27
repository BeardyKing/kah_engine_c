#ifndef DEFINES_H
#define DEFINES_H

//===FEATURES==================================================================
#define FEATURE_ON 1
#define FEATURE_OFF 0
#define CHECK_FEATURE(cond) _FEATURE_BOOL_TO_FEATURE(cond)

#if defined(_WIN32) || defined(_WIN64)
#define FEATURE_PLATFORM_WINDOWS FEATURE_ON
#define FEATURE_PLATFORM_LINUX   FEATURE_OFF
#define FEATURE_PLATFORM_APPLE   FEATURE_OFF
#elif defined(__linux__)
#define FEATURE_PLATFORM_WINDOWS FEATURE_OFF
#define FEATURE_PLATFORM_LINUX   FEATURE_ON
#define FEATURE_PLATFORM_APPLE   FEATURE_OFF
#elif defined(__APPLE__)
#define FEATURE_PLATFORM_WINDOWS FEATURE_OFF
#define FEATURE_PLATFORM_LINUX   FEATURE_OFF
#define FEATURE_PLATFORM_APPLE   FEATURE_ON
#endif

#define FEATURE_RUN_TESTS FEATURE_OFF
#define FEATURE_ASSERTS CHECK_FEATURE(KAH_DEBUG)
#define FEATURE_GFX_IMGUI FEATURE_ON
//=============================================================================

//===IMPL======================================================================
#define _FEATURE_BOOL_TO_FEATURE(x) _FEATURE_BOOL_TO_FEATURE_IMPL(x)
#define _FEATURE_BOOL_TO_FEATURE_IMPL(x) _FEATURE_##x
#define _FEATURE_1 FEATURE_ON
#define _FEATURE_0 FEATURE_OFF
//=============================================================================

//===COMPILER==================================================================
#if defined(_MSC_VER)
    #define CORE_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define CORE_FORCE_INLINE inline __attribute__((always_inline))
#else
    #define CORE_FORCE_INLINE inline
#endif
//=============================================================================
#endif // DEFINES_H
