#ifndef DEFINES_H
#define DEFINES_H

//===SIZES=====================================================================
#define KiB (1024)
#define MiB (KiB * KiB)
#define GiB (MiB * MiB)
#define TiB (GiB * GiB)
//=============================================================================

//===FEATURES==================================================================
#define FEATURE_ON ==
#define FEATURE_OFF !=
#define CHECK_FEATURE(FEATURE_STATE) (true FEATURE_STATE true)

#if defined(_WIN32)
#define FEATURE_PLATFORM_WINDOWS FEATURE_ON
#define FEATURE_PLATFORM_LINUX   FEATURE_OFF
#define FEATURE_PLATFORM_APPLE   FEATURE_OFF
#elif defined(__linux__) // #if defined(_WIN32)
#define FEATURE_PLATFORM_WINDOWS FEATURE_OFF
#define FEATURE_PLATFORM_LINUX   FEATURE_ON
#define FEATURE_PLATFORM_APPLE   FEATURE_OFF
#elif defined(__APPLE__) // #elif defined(__linux__)
#define FEATURE_PLATFORM_APPLE   FEATURE_OFF
#define FEATURE_PLATFORM_LINUX   FEATURE_OFF
#define FEATURE_PLATFORM_WINDOWS FEATURE_ON
#endif // #elif defined(__APPLE__)
//=============================================================================

#endif //DEFINES_H
