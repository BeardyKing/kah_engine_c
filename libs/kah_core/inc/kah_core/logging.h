#ifndef LOGGING_H
#define LOGGING_H

//===INCLUDES==================================================================
#include <kah_core/defines.h>

#include <stdint.h>
//=============================================================================

#define BIT_INDEX_VERBOSE 0
#define BIT_INDEX_INFO 1
#define BIT_INDEX_DEBUG 2
#define BIT_INDEX_WARNING 3
#define BIT_INDEX_ERROR 4
#define BIT_INDEX_CRITICAL 5

typedef void (*CorePrintf_cb)(uint32_t channel, const char* fmt, ...);
enum LogLevelBits{
    LOG_LEVEL_VERBOSE     = 1u << BIT_INDEX_VERBOSE,
    LOG_LEVEL_INFO        = 1u << BIT_INDEX_INFO,
    LOG_LEVEL_DEBUG       = 1u << BIT_INDEX_DEBUG,
    LOG_LEVEL_WARNING     = 1u << BIT_INDEX_WARNING,
    LOG_LEVEL_ERROR       = 1u << BIT_INDEX_ERROR,
    LOG_LEVEL_CRITICAL    = 1u << BIT_INDEX_CRITICAL
}typedef LogLevelBits;

#define LOG_LEVEL_ALL LOG_LEVEL_VERBOSE | LOG_LEVEL_INFO | LOG_LEVEL_DEBUG | LOG_LEVEL_WARNING | LOG_LEVEL_ERROR | LOG_LEVEL_CRITICAL
#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_STANDARD LOG_LEVEL_INFO | LOG_LEVEL_WARNING | LOG_LEVEL_ERROR | LOG_LEVEL_CRITICAL

enum LogChannelBits{
    LOG_CHANNEL_NONE      = 1u << 0u,
    LOG_CHANNEL_RUNTIME   = 1u << 1u,
    LOG_CHANNEL_SERVER    = 1u << 2u,
    LOG_CHANNEL_CORE      = 1u << 3u,
    LOG_CHANNEL_GFX       = 1u << 4u,
    LOG_CHANNEL_MATH      = 1u << 5u,
    LOG_CHANNEL_PIPELINE  = 1u << 6u,
    LOG_CHANNEL_CONVERTER = 1u << 7u,
    LOG_CHANNEL_MEMORY    = 1u << 8u,
    LOG_CHANNEL_DEBUG     = 1u << 31u,
    LOG_CHANNEL_ALL       = 0xffffffff
}typedef LogChannelBits;

CORE_FORCE_INLINE const char* log_level_to_string(LogLevelBits level){
    switch (level) {
        case LOG_LEVEL_VERBOSE:{ return "verbose"; }
        case LOG_LEVEL_INFO:{ return "info"; }
        case LOG_LEVEL_DEBUG:{ return "debug"; }
        case LOG_LEVEL_WARNING:{ return "warning"; }
        case LOG_LEVEL_ERROR:{ return "error"; }
        case LOG_LEVEL_CRITICAL:{ return "critical"; }
    }
    return "unknown";
}

CORE_FORCE_INLINE const char* log_channel_to_string(LogChannelBits channel){
    switch (channel) {
        case LOG_CHANNEL_NONE:{ return ""; }
        case LOG_CHANNEL_RUNTIME:{ return "runtime"; }
        case LOG_CHANNEL_SERVER:{ return "server"; }
        case LOG_CHANNEL_CORE:{ return "core"; }
        case LOG_CHANNEL_GFX:{ return "gfx"; }
        case LOG_CHANNEL_MATH:{ return "math"; }
        case LOG_CHANNEL_PIPELINE:{ return "pipeline"; }
        case LOG_CHANNEL_CONVERTER:{ return "converter"; }
        case LOG_CHANNEL_MEMORY:{ return "memory"; }
        case LOG_CHANNEL_DEBUG:{ return "debug"; }
        case LOG_CHANNEL_ALL:{ return "all"; }
    }
    return "unknown";
}
//===PUBLIC_STRUCTS============================================================
struct DebugLogInfo{
    LogLevelBits logLevel;
    CorePrintf_cb printf_cb;
} typedef DebugLogInfo;
//=============================================================================

#endif //LOGGING_H
