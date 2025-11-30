#include <kah_core/core_cvars.h>

#include <kah_core/window.h>

CoreCVars g_coreCvars = {};

void core_cvars_register() {
    g_coreCvars = (CoreCVars){
        .windowSize = vec2i_cvar_create("windowSize", C_VAR_DISK, (vec2i) { .x = 1024, .y = 768 }, (vec2i) { .x = 128, .y = 128 }, (vec2i) { .x = INT32_MAX, .y = INT32_MAX }),
        .windowPosition = vec2i_cvar_create("windowPosition", C_VAR_DISK, KAH_WINDOW_POSITION_CENTERED, (vec2i) { .x = INT32_MIN / 2, .y = INT32_MIN / 2 }, KAH_WINDOW_POSITION_CENTERED),
    };
}
