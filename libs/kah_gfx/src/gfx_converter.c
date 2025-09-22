#include <kah_gfx/gfx_converter.h>
#include <kah_gfx/gfx_logging.h>

#include <kah_converter/converter_interface.h>

#include <kah_core/assert.h>
// #include <kah_core/c_string.h>

extern ConverterLocations g_converterLocations;

// constexpr uint32_t SUPPORTED_CONVERTER_FORMATS_COUNT = 4;
// constexpr const char *SUPPORTED_CONVERTER_FORMATS[SUPPORTED_CONVERTER_FORMATS_COUNT]{
//         ".png",
//         ".tga",
//         ".jpg",
//         ".exr",
// };

#if CHECK_FEATURE(FEATURE_CONVERT_ON_DEMAND)
//===API================================================================================================================

void gfx_converter_create(const char *rawAssetDir, const char *targetAssetDir) {
    const bool initResult = converter_init(rawAssetDir, targetAssetDir);
    core_assert(initResult);

    constexpr bool optionForceReconvert = false;
    converter_option_set_ignore_cache(optionForceReconvert);

    gfx_log_verbose("Using on-demand converter lib\n");
    gfx_log_verbose("Source dir: %s\n", rawAssetDir);
    gfx_log_verbose("Target dir: %s\n", targetAssetDir);
    gfx_log_verbose("Shader compile : [TRUE]\n");
    gfx_log_verbose("texture convert : [TRUE]\n");
    gfx_log_verbose("Option force recompile : [%s]\n", optionForceReconvert ? "TRUE" : "FALSE");
}

bool gfx_convert_shader_spv(const char *localAssetPath) {
    const bool compileResult = convert_shader_spv(localAssetPath);
    core_assert_msg(compileResult, "err: gfx failed to compile shader %s \n", localAssetPath);
    return compileResult;
}

bool gfx_convert_texture_dds(const char *localAssetPath) {
    core_not_implemented();
    return false;
    // const char *delim = (c_str_search_reverse(localAssetPath, "."));
    // if (c_str_search_reverse(localAssetPath, ".") != nullptr) {
    //     core_sanity();
    // }
    // size_t copySize = delim - localAssetPath;
    // char fileNameNoExt[256] = {};
    // memcpy(fileNameNoExt, localAssetPath, copySize);
    //
    // char searchFileName[256] = {};
    // uint32_t foundIndex = {};
    // bool foundFile = false;
    // for (uint32_t i = 0; i < SUPPORTED_CONVERTER_FORMATS_COUNT; ++i) {
    //     memset(searchFileName, 0, sizeof(char) * KAH_FILESYSTEM_MAX_PATH);
    //     sprintf(searchFileName, "%s%s%s", &g_converterLocations.rawAssetDir[0], fileNameNoExt, SUPPORTED_CONVERTER_FORMATS[i]);
    //     foundFile = fs_file_exists(searchFileName);
    //     if (foundFile) {
    //         foundIndex = i;
    //         break;
    //     }
    // }
    //
    // core_assert(foundFile);
    // const char *ext = SUPPORTED_CONVERTER_FORMATS[foundIndex];
    // const bool compileResult = convert_texture_dds(fileNameNoExt, ext, strcmp(ext, ".exr") ? TextureFormat::BC6H : TextureFormat::BC7);
    // // This is quite a hack to get things working, as a concept.
    // // I think I should instead add some .meta file or some authored material file where I can grab converter info from.
    // return compileResult;
}

//======================================================================================================================
#endif //CHECK_FEATURE(FEATURE_CONVERT_ON_DEMAND)