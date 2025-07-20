#ifndef ASSERT_H
#define ASSERT_H

#if defined(__linux__) || defined(__APPLE__)
#include <signal.h>
#include <stdio.h>
#define CORE_DEBUG_BREAK() raise(SIGTRAP)
#elif defined(_WIN32)
#define CORE_DEBUG_BREAK() __debugbreak()
#endif

#endif //ASSERT_H
