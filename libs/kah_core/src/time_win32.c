#include <kah_core/defines.h>
#if CHECK_FEATURE(FEATURE_PLATFORM_WINDOWS)

//===INCLUDES==================================================================
#include <kah_core/time.h>
#include <windows.h>
//=============================================================================

//===INTERNAL_STRUCTS==========================================================
struct Time {
    double timeOnStartUp;
    double lastTime;
    double currentTime;
    double timeScaleDelta;
    double timeScale;
    double frequency;
    uint32_t frameCount;
    double deltaTime;
} typedef Time;

static Time s_time = (Time){};
//=============================================================================

//===API=======================================================================
double time_delta_d() {
    return s_time.deltaTime;
}

float time_delta_f(){
    return (float)s_time.deltaTime;
}

double time_current() {
    return s_time.currentTime - s_time.timeOnStartUp;
}

uint32_t time_frame_count() {
    return s_time.frameCount;
}

void time_tick() {
    LARGE_INTEGER timeNow;

    QueryPerformanceCounter(&timeNow);

    s_time.frameCount += 1;
    s_time.lastTime = s_time.currentTime;
    s_time.currentTime = (double) timeNow.QuadPart / s_time.frequency;
    s_time.deltaTime = (s_time.currentTime - s_time.lastTime) * (s_time.timeScale / 1000.0);
}
//=============================================================================

//===INIT_&_SHUTDOWN===========================================================
void time_create() {
    LARGE_INTEGER now;
    LARGE_INTEGER frequency;

    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&frequency);

    s_time.timeOnStartUp = (double) now.QuadPart / (double) frequency.QuadPart;
    s_time.lastTime = (double) now.QuadPart / (double) frequency.QuadPart;
    s_time.currentTime = (double) now.QuadPart / (double) frequency.QuadPart;
    s_time.timeScaleDelta = 1.0;
    s_time.timeScale = 1000.0;
    s_time.frequency = (double) frequency.QuadPart;
    s_time.frameCount = 0;
}

void time_cleanup() {
    s_time = (Time){};
}
//=============================================================================F

#endif