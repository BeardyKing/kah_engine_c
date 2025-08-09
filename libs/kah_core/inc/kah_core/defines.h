#ifndef DEFINES_H
#define DEFINES_H

//===FEATURES==================================================================
#define FEATURE_ON ==
#define FEATURE_OFF !=
#define CHECK_FEATURE(FEATURE_STATE) (true FEATURE_STATE true)

#if defined(_WIN32) || defined(_WIN64)
#define FEATURE_PLATFORM_WINDOWS FEATURE_ON
#define FEATURE_PLATFORM_LINUX   FEATURE_OFF
#define FEATURE_PLATFORM_APPLE   FEATURE_OFF
#elif defined(__linux__)
#define FEATURE_PLATFORM_WINDOWS FEATURE_OFF
#define FEATURE_PLATFORM_LINUX   FEATURE_ON
#define FEATURE_PLATFORM_APPLE   FEATURE_OFF
#elif defined(__APPLE__)
#define FEATURE_PLATFORM_APPLE   FEATURE_OFF
#define FEATURE_PLATFORM_LINUX   FEATURE_OFF
#define FEATURE_PLATFORM_WINDOWS FEATURE_ON
#endif

#define FEATURE_RUN_TESTS FEATURE_OFF

//=============================================================================

#endif //DEFINES_H
