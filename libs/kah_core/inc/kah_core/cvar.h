#ifndef CVAR_H
#define CVAR_H

#include <kah_core/cvar.h>
#include <kah_core/defines.h>
#include <kah_math/vec2.h>

#include <stdbool.h>

#define CVAR_NAME_SIZE 32

struct i32_cvar_t   { int32_t   current; int32_t    reset; int32_t  fallback; int32_t   min; int32_t    max;    } typedef i32_cvar_t;
struct i64_cvar_t   { int64_t   current; int64_t    reset; int64_t  fallback; int64_t   min; int64_t    max;    } typedef i64_cvar_t;
struct vec2i_cvar_t { vec2i     current; vec2i      reset; vec2i    fallback; vec2i     min; vec2i      max;    } typedef vec2i_cvar_t;
struct u32_cvar_t   { uint64_t  current; uint64_t   reset; uint64_t fallback; uint64_t  min; uint64_t   max;    } typedef u32_cvar_t;
struct u64_cvar_t   { uint32_t  current; uint32_t   reset; uint32_t fallback; uint32_t  min; uint32_t   max;    } typedef u64_cvar_t;
struct vec2u_cvar_t { vec2u     current; vec2u      reset; vec2u    fallback; vec2u     min; vec2u      max;    } typedef vec2u_cvar_t;
struct f32_cvar_t   { float     current; float      reset; float    fallback; float     min; float      max;    } typedef f32_cvar_t;
struct f64_cvar_t   { double    current; double     reset; double   fallback; double    min; double     max;    } typedef f64_cvar_t;
struct bool_cvar_t  { bool      current; bool       reset; bool     fallback;                                   } typedef bool_cvar_t;

enum CVarLoadType {
    C_VAR_RUNTIME,
    C_VAR_DISK,
}typedef CVarLoadType;

i32_cvar_t *i32_cvar_create(const char* name, CVarLoadType loadType, int32_t defaultValue, int32_t min, int32_t max);
CORE_FORCE_INLINE int32_t i32_cvar_get(const i32_cvar_t* cVar) {return cVar->current;};
// __forceinline void i32_cvar_set(i32_cvar_t *cVar, int32_t inVal){}
// __forceinline void i32_cvar_reset(i32_cvar_t *cVar){}
// __forceinline void i32_cvar_set_reset(i32_cvar_t *cVar, int32_t inVal){}
// __forceinline void i32_cvar_reset_to_default(i32_cvar_t* cVar){}

//
void cvar_save_to_disk(const char* path);
//
void cvar_create(const char* path);
void cvar_cleanup();
//

#endif //CVAR_H
