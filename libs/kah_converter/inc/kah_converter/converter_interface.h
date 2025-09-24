#ifndef CONVERTER_INTERFACE_H
#define CONVERTER_INTERFACE_H

#include <kah_core/filesystem.h>
#include <kah_core/texture_formats.h>

//===INIT/SHUTDOWN=============================================================
bool converter_init(const char *rawAssetDir, const char *targetAssetDir);
//=============================================================================

//===API=======================================================================
bool convert_shader_spv(const char *localAssetDir);
bool convert_texture_dds(const char *localAssetDir, const char *inFileFormat, CoreTextureFormat format);

bool converter_cache_check_needs_convert(const char *toPath, const char *fromPath);
void converter_option_set_ignore_cache(bool ignoreCacheOption);
//=============================================================================

//===PUBLIC_STRUCTS============================================================
struct ConverterLocations {
    char rawAssetDir[KAH_FILESYSTEM_MAX_PATH];
    char targetAssetDir[KAH_FILESYSTEM_MAX_PATH];

    char vulkanSDKDir[KAH_FILESYSTEM_MAX_PATH];
    char compressonatorSDKDir[KAH_FILESYSTEM_MAX_PATH];

    char compressonatorCLI[KAH_FILESYSTEM_MAX_PATH];
    char glslValidatorCLI[KAH_FILESYSTEM_MAX_PATH];
}typedef ConverterLocations;

struct ConverterOptions {
    bool ignoreConvertCache;
}typedef ConverterOptions;
//=============================================================================

#endif //CONVERTER_INTERFACE_H
