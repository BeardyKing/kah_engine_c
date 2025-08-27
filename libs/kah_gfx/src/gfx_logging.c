//===INCLUDES==================================================================
#include <kah_gfx/gfx_logging.h>

#include <kah_core/logging.h>
#include <kah_core/bit_array.h>

#include <stdarg.h>
#include <stdio.h>
//=============================================================================

//===INTERNAL_STRUCTS==========================================================
static DebugLogInfo s_gfxLogging = (DebugLogInfo){
    .logLevel = LOG_LEVEL_STANDARD,
    .printf_cb = nullptr
};
//=============================================================================

//===INTERNAL_FUNCTIONS========================================================
static void _internal_gfx_log(LogLevelBits level, const char* fmt, va_list args) {
    if (s_gfxLogging.printf_cb) {
        s_gfxLogging.printf_cb( LOG_CHANNEL_GFX, fmt, args);
    }
    else {
        printf("[gfx][%s] ", log_level_to_string(level));
        vprintf(fmt, args);
    }
}
//=============================================================================

//===API=======================================================================
void gfx_log_verbose(const char* fmt, ...) {
    if (!u32_is_bit_set(s_gfxLogging.logLevel, BIT_INDEX_VERBOSE)) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    _internal_gfx_log(LOG_LEVEL_VERBOSE, fmt, args);
    va_end(args);
}

void gfx_log_info(const char* fmt, ...) {
    if (!u32_is_bit_set(s_gfxLogging.logLevel, BIT_INDEX_INFO)) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    _internal_gfx_log(LOG_LEVEL_INFO, fmt, args);
    va_end(args);
}

void gfx_log_debug(const char* fmt, ...) {
    if (!u32_is_bit_set(s_gfxLogging.logLevel, BIT_INDEX_DEBUG)){
        return;
    }
    va_list args;
    va_start(args, fmt);
    _internal_gfx_log(LOG_LEVEL_DEBUG, fmt, args);
    va_end(args);
}

void gfx_log_warning(const char* fmt, ...) {
    if (!u32_is_bit_set(s_gfxLogging.logLevel, BIT_INDEX_WARNING)){
        return;
    }
    va_list args;
    va_start(args, fmt);
    _internal_gfx_log(LOG_LEVEL_WARNING, fmt, args);
    va_end(args);
}

void gfx_log_error(const char* fmt, ...) {
    if (!u32_is_bit_set(s_gfxLogging.logLevel, BIT_INDEX_ERROR)){
        return;
    }
    va_list args;
    va_start(args, fmt);
    _internal_gfx_log(LOG_LEVEL_ERROR, fmt, args);
    va_end(args);
}

void gfx_log_critical(const char* fmt, ...) {
    if (!u32_is_bit_set(s_gfxLogging.logLevel, BIT_INDEX_CRITICAL)){
        return;
    }
    va_list args;
    va_start(args, fmt);
    _internal_gfx_log(LOG_LEVEL_CRITICAL, fmt, args);
    va_end(args);
}

void gfx_set_callbacks(CorePrintf_cb cb){
    s_gfxLogging.printf_cb = cb;
}
//=============================================================================