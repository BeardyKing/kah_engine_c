#include <kah_converter/converter_interface.h>

#include <kah_core/defines.h>
#include <kah_core/assert.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//===GLOBAL_STRUCTS============================================================
ConverterLocations g_converterLocations;
ConverterOptions g_converterOptions;
//=============================================================================

//===INTERNAL_FUNCTIONS========================================================
static void c_str_n_copy(char *dst, const char *src, size_t max) {
    core_assert(src != nullptr);
    strncpy(dst, src, max - 1);
    dst[max - 1] = '\0';
}

static void c_str_n_copy_env(char *dst, const char *envName, size_t max) {
    const char *val = getenv(envName);
    core_assert_msg(val != nullptr,"err: environment variable '$%s' is not set", envName);
    c_str_n_copy(dst, val, max);
}
//=============================================================================

//===API=======================================================================
bool converter_init(const char *rawAssetDir, const char *targetAssetDir) {
    c_str_n_copy(g_converterLocations.rawAssetDir, rawAssetDir, KAH_FILESYSTEM_MAX_PATH);
    c_str_n_copy(g_converterLocations.targetAssetDir, targetAssetDir, KAH_FILESYSTEM_MAX_PATH);

    c_str_n_copy_env(g_converterLocations.compressonatorSDKDir, "COMPRESSONATOR_ROOT", KAH_FILESYSTEM_MAX_PATH);
    c_str_n_copy_env(g_converterLocations.vulkanSDKDir, "VULKAN_SDK", KAH_FILESYSTEM_MAX_PATH);

#if CHECK_FEATURE(FEATURE_PLATFORM_WINDOWS)
    snprintf(g_converterLocations.compressonatorCLI, KAH_FILESYSTEM_MAX_PATH,"%s\\bin\\CLI\\compressonatorcli.exe", g_converterLocations.compressonatorSDKDir);
    snprintf(g_converterLocations.glslValidatorCLI, KAH_FILESYSTEM_MAX_PATH, "%s\\Bin\\glslangValidator.exe", g_converterLocations.vulkanSDKDir);
#elif CHECK_FEATURE(FEATURE_PLATFORM_LINUX)
    snprintf(g_converterLocations.compressonatorCLI, KAH_FILESYSTEM_MAX_PATH,"%s\\bin\\CLI\\compressonatorcli", g_converterLocations.compressonatorSDKDir);
    snprintf(g_converterLocations.glslValidatorCLI, KAH_FILESYSTEM_MAX_PATH, "%s\\Bin\\glslangValidator", g_converterLocations.vulkanSDKDir);
#endif

    if (g_converterLocations.rawAssetDir[0] == '\0' || g_converterLocations.targetAssetDir[0] == '\0') {
        core_sanity();
        return false;
    }
    return true;
}

void converter_option_set_ignore_cache(const bool ignoreCacheOption) {
    g_converterOptions.ignoreConvertCache = ignoreCacheOption;
    if (g_converterOptions.ignoreConvertCache) {
        printf("option set - ignoring converter cache.\n");
    }
}
//=============================================================================
