#include <kah_core/defines.h>

#if CHECK_FEATURE(FEATURE_PLATFORM_WINDOWS)

#include <kah_core/filesystem.h>
#include <kah_core/c_string.h>

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <string.h>
#include <sys/stat.h>
#include <direct.h>

//===API=======================================================================
bool fs_dir_make(const char *path) {
    return (_mkdir(path) == 0);
}

bool fs_dir_remove(const char *path) {
    return (_rmdir(path) == 0);
}

bool fs_file_exists(const char *path) {
    struct stat buffer = (struct stat){};
    return (stat(path, &buffer) == 0);
}

size_t fs_file_size(const char *path) {
    struct stat buffer = (struct stat){};
    stat(path, &buffer);
    return (size_t)buffer.st_size;
}

#define KAH_FILESYSTEM_RECURSIVE_MAX_CPY_SIZE (sizeof(char) * KAH_FILESYSTEM_MAX_PATH)

static bool fs_mkdir_recursive_internal(const char path[KAH_FILESYSTEM_MAX_PATH]) {
    char currPath[KAH_FILESYSTEM_MAX_PATH] = {};
    memcpy(currPath, path, KAH_FILESYSTEM_RECURSIVE_MAX_CPY_SIZE);

    char *delimLocation = strrchr(currPath, '/');
    if (!fs_file_exists(currPath) && delimLocation != NULL) {
        *delimLocation = '\0';
        fs_mkdir_recursive_internal(currPath);
    } else {
        return false;
    }

    return fs_dir_make(currPath);
}

//Note: This function can either take in a file path or a directory path
//      When using a directory path you MUST include a trailing '/'
bool fs_dir_make_recursive(const char* path) {
    if (c_str_empty(path)) {
        return false;
    }
    return fs_mkdir_recursive_internal(path);
}           

bool fs_file_is_absolute(const char *inPath) {
    const size_t pathLength = strlen(inPath);
    if (inPath == NULL || pathLength == 0) {
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
    if (inPath == NULL || strlen(inPath) > MAX_PATH - 3) {
        return KAH_FILESYSTEM_INVALID;
    }

    int32_t outFileCount = 0;

    TCHAR szDir[MAX_PATH];
    StringCchCopy(szDir, MAX_PATH, inPath);
    StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(szDir, &ffd);
    {
        [[maybe_unused]] int32_t directoryCount = 0;
        if (INVALID_HANDLE_VALUE == hFind) {
            return KAH_FILESYSTEM_INVALID;
        }

        do {
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                directoryCount++;
            }
            else {
                outFileCount++;
            }
        } while (FindNextFile(hFind, &ffd) != 0);
    }
    FindClose(hFind);

    return outFileCount;
}

bool fs_dir_get_filenames(const char* inPath, char* inOutBuffer, const int32_t inFileCount) {
    if (inOutBuffer == NULL || inFileCount == 0) {
        return false;
    }

    TCHAR szDir[MAX_PATH];
    StringCchCopy(szDir, MAX_PATH, inPath);
    StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(szDir, &ffd);
    {
        if (INVALID_HANDLE_VALUE == hFind) {
            return KAH_FILESYSTEM_INVALID;
        }

        int32_t fileCount = 0;
        do {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                memcpy(inOutBuffer + (sizeof(char) * fileCount * MAX_PATH), ffd.cFileName, MAX_PATH);
                fileCount++;

                if (fileCount > inFileCount) {
                    memset(inOutBuffer, 0, inFileCount * (sizeof(char) * fileCount * MAX_PATH));
                    return false;
                }
            }
        } while (FindNextFile(hFind, &ffd) != 0);
    }
    FindClose(hFind);

    return true;
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

