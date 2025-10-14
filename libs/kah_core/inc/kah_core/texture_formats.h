#ifndef TEXTURE_FORMATS_H
#define TEXTURE_FORMATS_H

//===INCLUDES==================================================================
#include <stdint.h>

#include <kah_core/allocators.h>
//=============================================================================

//===DEFINES===================================================================
#define CORE_RAW_IMAGE_MIP_MAX 16
//=============================================================================

//===STRUCTS===================================================================
enum CoreTextureFormat {
    CORE_TEXTURE_FORMAT_INVALID = 0,
    CORE_TEXTURE_FORMAT_RGBA8,
    CORE_TEXTURE_FORMAT_RGBA16,

    CORE_TEXTURE_FORMAT_BC1RGBA,
    CORE_TEXTURE_FORMAT_BC2,
    CORE_TEXTURE_FORMAT_BC3,
    CORE_TEXTURE_FORMAT_BC4,
    CORE_TEXTURE_FORMAT_BC5,
    CORE_TEXTURE_FORMAT_BC6H,
    CORE_TEXTURE_FORMAT_BC7,
}typedef CoreTextureFormat;

struct CoreRawImage{
    CoreTextureFormat textureFormat;
    uint32_t mipMapCount;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    size_t dataSize;
    size_t mipDataSizes[CORE_RAW_IMAGE_MIP_MAX];
    AllocInfo* imageData;
}typedef CoreRawImage;
//=============================================================================

#endif //TEXTURE_FORMATS_H
