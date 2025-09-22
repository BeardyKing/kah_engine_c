#include <kah_core/defines.h>

#if CHECK_FEATURE(FEATURE_PLATFORM_WINDOWS)

#include <kah_core/filesystem.h>
#include <kah_core/c_string.h>

#include <string.h>
#include <sys/stat.h>
#include <direct.h>

//===API=======================================================================
bool fs_mkdir(const char *path) {
    return (_mkdir(path) == 0);
}

bool fs_rmdir(const char *path) {
    return (_rmdir(path) == 0);
}

bool fs_file_exists(const char *path) {
    struct stat buffer = (struct stat){};
    return (stat(path, &buffer) == 0);
}

size_t fs_file_size(const char *path) {
    struct stat buffer = (struct stat){};
    stat(path, &buffer);
    return buffer.st_size;
}

bool fs_mkdir_recursive_internal(const char path[KAH_FILESYSTEM_MAX_PATH]) {
    constexpr size_t cpySize = sizeof(char) * KAH_FILESYSTEM_MAX_PATH;
    char currPath[KAH_FILESYSTEM_MAX_PATH] = {};
    memcpy(currPath, path, cpySize);

    char *delimLocation = strrchr(currPath, '/');
    if (!fs_file_exists(currPath) && delimLocation != nullptr) {
        *delimLocation = '\0';
        fs_mkdir_recursive_internal(currPath);
    } else {
        return false;
    }

    return fs_mkdir(currPath);
}

//Note: This function can either take in a file path or a directory path
//      When using a directory path you MUST include a trailing '/'
bool fs_mkdir_recursive(const char path[KAH_FILESYSTEM_MAX_PATH]) {
    if (c_str_empty(path)) {
        return false;
    }
    return fs_mkdir_recursive_internal(path);
}

bool fs_file_is_absolute(const char *inPath) {
    const size_t pathLength = strlen(inPath);
    if (inPath == nullptr || pathLength == 0) {
        return false;
    }

    if ((pathLength > 2 && inPath[1] == ':' && inPath[2] == '\\') ||
        (pathLength > 2 && inPath[1] == ':' && inPath[2] == '/') ||
        (pathLength > 1 && inPath[0] == '\\' && inPath[1] == '\\') ||
        (pathLength > 1 && inPath[0] == '\\' && inPath[1] == '/')) {
        return true;
    }

    return false;
}
//=============================================================================

#endif

