#include <kah_converter/converter_interface.h>

#include <kah_core/filesystem.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//===GLOBAL_STRUCTS============================================================
extern ConverterLocations g_converterLocations;
//=============================================================================

//===API=======================================================================
bool convert_shader_spv(const char *localAssetDir) {
    char inPath[KAH_FILESYSTEM_MAX_PATH * 2];
    char outPath[KAH_FILESYSTEM_MAX_PATH * 2];

    snprintf(inPath, sizeof(inPath), "%s%s", g_converterLocations.rawAssetDir, localAssetDir);
    snprintf(outPath, sizeof(outPath), "%s%s", g_converterLocations.targetAssetDir, localAssetDir);

    fs_dir_make_recursive(outPath);

    if (!fs_file_exists(inPath)) {
        return false;
    }

    if (converter_cache_check_needs_convert(outPath, inPath)) {
        char cmd[KAH_FILESYSTEM_MAX_PATH * 4];
        printf("shader: %s\n", outPath);
        snprintf(cmd, sizeof(cmd), "%s -V -o %s %s", g_converterLocations.glslValidatorCLI, outPath, inPath);
        system(cmd);
    }

    return true;
}
//=============================================================================
