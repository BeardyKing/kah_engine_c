#ifndef GFX_LOGGING_H
#define GFX_LOGGING_H
//===INCLUDES==================================================================
#include <kah_core/logging.h>
//=============================================================================

//===API=======================================================================
void gfx_log_verbose(const char* fmt, ...);
void gfx_log_info(const char* fmt, ...);
void gfx_log_debug(const char* fmt, ...);
void gfx_log_warning(const char* fmt, ...);
void gfx_log_error(const char* fmt, ...);
void gfx_log_critical(const char* fmt, ...);

void gfx_set_callbacks(CorePrintf_cb cb); // not required, useful for capturing output & redirecting file/app console
//=============================================================================

#endif //GFX_LOGGING_H
