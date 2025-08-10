#ifndef VEC4_H
#define VEC4_H

//===INCLUDES==================================================================
#include <stdint.h>
//=============================================================================

//===PUBLIC_STRUCTS============================================================
union vec4i{
    struct{ int32_t x, y, z, w; };
    struct{ int32_t r, g, b, a; };
} typedef vec4i;

union vec4u{
    struct{ uint32_t x, y, z, w; };
    struct{ uint32_t r, g, b, a; };
} typedef vec4u;

union vec4f{
    struct{ float x, y, z, w; };
    struct{ float r, g, b, a; };
} typedef vec4f;

union vec4d{
    struct{ double x, y, z, w; };
    struct{ double r, g, b, a; };
} typedef vec4d;
//=============================================================================

//===API=======================================================================
uint32_t pack_vec4f_to_uint32_t(const vec4f* vec);
vec4f    unpack_uint32_t_to_vec4f(uint32_t packedValue);
//=============================================================================


#endif //VEC4_H
