#ifndef CVAR_H
#define CVAR_H

//===INCLUDES==================================================================
#include <kah_core/cvar.h>
#include <kah_core/defines.h>

#include <kah_math/vec2.h>
#include <kah_math/utils.h>

#include <stdbool.h>
//=============================================================================

//===TYPES=====================================================================
struct bool_cvar_t  { bool      current; bool       reset; bool     fallback;                                   } typedef bool_cvar_t;
struct i32_cvar_t   { int32_t   current; int32_t    reset; int32_t  fallback; int32_t   min; int32_t    max;    } typedef i32_cvar_t;
struct u32_cvar_t   { uint32_t  current; uint32_t   reset; uint32_t fallback; uint32_t  min; uint32_t   max;    } typedef u32_cvar_t;
struct f32_cvar_t   { float     current; float      reset; float    fallback; float     min; float      max;    } typedef f32_cvar_t;
struct vec2i_cvar_t { vec2i     current; vec2i      reset; vec2i    fallback; vec2i     min; vec2i      max;    } typedef vec2i_cvar_t;

#define CVAR_NAME_SIZE 32
enum CVarLoadType {
    C_VAR_RUNTIME,
    C_VAR_DISK,
}typedef CVarLoadType;
//=============================================================================

//===API=======================================================================
bool_cvar_t* bool_cvar_create(const char* name, CVarLoadType loadType, bool defaultValue);
CORE_FORCE_INLINE bool bool_cvar_get(bool_cvar_t* cVar)                     { return cVar->current; };
CORE_FORCE_INLINE void bool_cvar_set(bool_cvar_t* cVar, bool inVal)         { cVar->current = inVal; }
CORE_FORCE_INLINE void bool_cvar_reset(bool_cvar_t* cVar)                   { cVar->current = cVar->reset; }
CORE_FORCE_INLINE void bool_cvar_set_reset(bool_cvar_t* cVar, bool inVal)   { cVar->reset = inVal; }
CORE_FORCE_INLINE void bool_cvar_reset_to_default(bool_cvar_t* cVar)        { cVar->current = cVar->reset; };

i32_cvar_t* i32_cvar_create(const char* name, CVarLoadType loadType, int32_t defaultValue, int32_t min, int32_t max);
CORE_FORCE_INLINE int32_t i32_cvar_get(i32_cvar_t* cVar)                    { return cVar->current; };
CORE_FORCE_INLINE void i32_cvar_set(i32_cvar_t* cVar, int32_t inVal)        { cVar->current = clamp_i32( inVal, cVar->min, cVar->max ); }
CORE_FORCE_INLINE void i32_cvar_reset(i32_cvar_t* cVar)                     { cVar->current = cVar->reset; }
CORE_FORCE_INLINE void i32_cvar_set_reset(i32_cvar_t* cVar, int32_t inVal)  { cVar->reset = clamp_i32( inVal, cVar->min, cVar->max ); }
CORE_FORCE_INLINE void i32_cvar_reset_to_default(i32_cvar_t* cVar)          { cVar->current = cVar->reset; };

u32_cvar_t* u32_cvar_create(const char* name, CVarLoadType loadType, uint32_t defaultValue, uint32_t min, uint32_t max);
CORE_FORCE_INLINE uint32_t u32_cvar_get(u32_cvar_t* cVar)                   { return cVar->current; };
CORE_FORCE_INLINE void u32_cvar_set(u32_cvar_t* cVar, uint32_t inVal)       { cVar->current = clamp_u32(inVal, cVar->min, cVar->max); }
CORE_FORCE_INLINE void u32_cvar_reset(u32_cvar_t* cVar)                     { cVar->current = cVar->reset; }
CORE_FORCE_INLINE void u32_cvar_set_reset(u32_cvar_t* cVar, uint32_t inVal) { cVar->reset = clamp_u32(inVal, cVar->min, cVar->max); }
CORE_FORCE_INLINE void u32_cvar_reset_to_default(u32_cvar_t* cVar)          { cVar->current = cVar->reset; };


void cvar_serialise_disk_vars();
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void cvar_create(const char* path);
void cvar_cleanup();
//=============================================================================

#endif //CVAR_H
