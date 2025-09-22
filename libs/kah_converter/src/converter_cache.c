#include <stdio.h>
#include <kah_converter/converter_interface.h>

#include <sys/stat.h>

//===GLOBAL_STRUCTS============================================================
extern ConverterOptions g_converterOptions;
//=============================================================================

//===API=======================================================================
bool converter_cache_check_needs_convert(const char *toPath, const char *fromPath) {
    if (g_converterOptions.ignoreConvertCache) {
        return true;
    }
    struct stat toFileStat = (struct stat){};
    struct stat fromFileStat = (struct stat){};

    stat(toPath, &toFileStat);
    stat(fromPath, &fromFileStat);

    if (toFileStat.st_mtime > fromFileStat.st_mtime) {
        printf("cached skipping convert: %s \n", fromPath);
        return false;
    }
    return true;
}
//=============================================================================