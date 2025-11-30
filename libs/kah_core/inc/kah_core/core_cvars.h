#ifndef CORE_CVARS_H
#define CORE_CVARS_H

#include <kah_core/cvar.h>

struct CoreCVars {
    vec2i_cvar_t* windowSize;
    vec2i_cvar_t* windowPosition;
} typedef CoreCVars;

extern CoreCVars g_coreCvars;

void core_cvars_register();

#endif //CORE_CVARS_H
