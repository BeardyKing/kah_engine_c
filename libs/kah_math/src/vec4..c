//===INCLUDES==================================================================
#include <kah_math/vec4.h>
//=============================================================================

//===API=======================================================================
uint32_t pack_vec4f_to_uint32_t(const vec4f* vec){
    uint32_t packedValue = {0};
    packedValue |= (uint32_t)((uint8_t)(vec->x * 255.0f)) << 24;
    packedValue |= (uint32_t)((uint8_t)(vec->y * 255.0f)) << 16;
    packedValue |= (uint32_t)((uint8_t)(vec->z * 255.0f)) << 8;
    packedValue |= (uint32_t)((uint8_t)(vec->w * 255.0f)) << 0;
    return packedValue;
}

vec4f unpack_uint32_t_to_vec4f(uint32_t packedValue){
    return (vec4f){
        .x = (float)(packedValue >> 24 & 0xFF) / 255.0f,
        .y = (float)(packedValue >> 16 & 0xFF) / 255.0f,
        .z = (float)(packedValue >> 8 & 0xFF) / 255.0f,
        .w = (float)(packedValue >> 0 & 0xFF) / 255.0f,
    };
}
//=============================================================================
