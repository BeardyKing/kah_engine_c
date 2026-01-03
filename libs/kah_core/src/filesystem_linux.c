#include <kah_core/defines.h>

#if CHECK_FEATURE(FEATURE_PLATFORM_LINUX)

#include <kah_core/filesystem.h>
#include <kah_core/c_string.h>
#include <kah_core/assert.h>

#include <string.h>

#include <sys/stat.h>
#include <unistd.h>

//===API=======================================================================
bool fs_mkdir(const char *path) {
    return mkdir(path, 0777) == 0;
}

bool fs_rmdir(const char *path) {
    return rmdir(path) == 0;
}

bool fs_file_exists(const char *path) {
    struct stat buffer;
    return stat(path, &buffer) == 0;
}

size_t fs_file_size(const char *path) {
    struct stat buffer;
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
bool fs_dir_make_recursive(const char *path) {
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


int32_t fs_dir_file_count(const char* inPath) {
    core_not_implemented();
    return 0;
}

bool fs_dir_get_filenames(const char* inPath, char* inOutBuffer, const int32_t inFileCount) {
    core_not_implemented();
    return false;
}
//=============================================================================
char s_exePath[KAH_FILESYSTEM_MAX_PATH];
char s_exeDir[KAH_FILESYSTEM_MAX_PATH];

const char* fs_exe_path(void) {
    return s_exePath;
}

const char* fs_exe_dir(void) {
    return s_exeDir;
}

//===INIT/SHUTDOWN=============================================================
void fs_create(int argc, char** argv) {
    memcpy(s_exePath, argv[0], strlen(argv[0]) * sizeof(char));
    c_str_remove_file_from_path(s_exePath, s_exeDir);
}

void fs_cleanup() {
    memset(s_exePath, 0, sizeof(char) * KAH_FILESYSTEM_MAX_PATH);
    memset(s_exeDir, 0, sizeof(char) * KAH_FILESYSTEM_MAX_PATH);
}
//=============================================================================

#endif
