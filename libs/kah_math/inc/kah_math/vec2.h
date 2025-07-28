#ifndef VEC2_H
#define VEC2_H

//===INCLUDES==================================================================
#include <stdint.h>
//=============================================================================

//===PUBLIC_STRUCTS============================================================
union vec2i{
    struct{ int32_t x, y; };
    struct{ int32_t u, v; };
} typedef vec2i;

union vec2u{
    struct{ uint32_t x, y; };
    struct{ uint32_t u, v; };
} typedef vec2u;

union vec2f{
    struct{ float x, y; };
    struct{ float u, v; };
} typedef vec2f;

union vec2d{
    struct{ double x, y; };
    struct{ double u, v; };
}typedef vec2d;
//=============================================================================


#endif //VEC2_H
