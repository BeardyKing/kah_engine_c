#ifndef GFX_CONVERTER_H
#define GFX_CONVERTER_H

#include <kah_core/defines.h>
#include <stdbool.h>

#if CHECK_FEATURE(FEATURE_CONVERT_ON_DEMAND)
//===API=======================================================================
void gfx_converter_create(const char *rawAssetDir, const char *targetAssetDir);
bool gfx_convert_shader_spv(const char *localAssetPath);
bool gfx_convert_texture_dds(const char *localAssetPath);
//=============================================================================
#endif //CHECK_FEATURE(FEATURE_CONVERT_ON_DEMAND)


#endif //GFX_CONVERTER_H
