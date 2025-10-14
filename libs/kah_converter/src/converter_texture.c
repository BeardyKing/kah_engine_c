#include <stdio.h>
#include <stdlib.h>

#include "converter_interface.h"
#include "kah_core/assert.h"
#include "kah_core/texture_formats.h"

//===INTERNAL_STRUCTS===================================================================================================
extern ConverterLocations g_converterLocations;
//======================================================================================================================

const char* texture_format_to_string(const CoreTextureFormat textureFormat) {
    switch (textureFormat) {
        case CORE_TEXTURE_FORMAT_RGBA8:   return "RGBA8";
        case CORE_TEXTURE_FORMAT_RGBA16:  return "RGBA16";

        case CORE_TEXTURE_FORMAT_BC1RGBA: return "BC1RGBA";
        case CORE_TEXTURE_FORMAT_BC2:     return "BC2";
        case CORE_TEXTURE_FORMAT_BC3:     return "BC3";
        case CORE_TEXTURE_FORMAT_BC4:     return "BC4";
        case CORE_TEXTURE_FORMAT_BC5:     return "BC5";
        case CORE_TEXTURE_FORMAT_BC6H:    return "BC6H";
        case CORE_TEXTURE_FORMAT_BC7:     return "BC7";
        default:;
    };
    core_not_implemented();
    return "ERR_UNKNOWN_FORMAT";
}

constexpr char CONVERTER_MAX_MIP_COUNT[] = "-miplevels 20";
constexpr char CONVERTER_ENCODE_TYPE[] = "-EncodeWith GPU";
constexpr char CONVERTER_THREAD_COUNT[] = "-EncodeWith GPU";
constexpr char CONVERTER_2X2_MIPS[] = "-mipsize 2";
constexpr char CONVERTER_4X4_MIPS[] = "-mipsize 4";

//===API================================================================================================================
bool convert_texture_dds(const char *localAssetDir, const char *inFileFormat, const CoreTextureFormat format) {
    char inPath[KAH_FILESYSTEM_MAX_PATH * 2];
    char outPath[KAH_FILESYSTEM_MAX_PATH * 2];

    snprintf(inPath, sizeof(inPath), "%s%s%s", g_converterLocations.rawAssetDir, localAssetDir, inFileFormat);
    snprintf(outPath, sizeof(outPath), "%s%s.dds", g_converterLocations.targetAssetDir, localAssetDir);

    fs_mkdir_recursive(outPath);

    if (!fs_file_exists(inPath)) {
        printf("no file at path: %s \n", inPath);
        return false;
    }

    if (converter_cache_check_needs_convert(outPath, inPath)) {
        char cmd[KAH_FILESYSTEM_MAX_PATH * 4];
        snprintf(cmd, sizeof(cmd),
                "%s -fd %s %s %s %s %s %s %s",
                g_converterLocations.compressonatorCLI,
                texture_format_to_string(format),
                CONVERTER_THREAD_COUNT,
                CONVERTER_ENCODE_TYPE,
                false ? CONVERTER_4X4_MIPS : "", // GPU encoders work on 4x4 which isn't ideal as we don't get as many mip maps. likely toggle on HIGH quality settings
                CONVERTER_MAX_MIP_COUNT,
                inPath,
                outPath
        );

        printf("texture: %s \n", outPath);
        system(cmd);
    }
    return true;
}
//======================================================================================================================
