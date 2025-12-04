#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//===DEFINES===================================================================
#define KAH_FILESYSTEM_MAX_PATH 260 //MAX_PATH
#define KAH_FILESYSTEM_INVALID -1
//=============================================================================

//===API=======================================================================
bool    fs_dir_make(const char *path);
bool    fs_dir_make_recursive(const char *path);
bool    fs_dir_remove(const char *path);
bool    fs_dir_get_filenames(const char* inPath, char* inOutBuffer, const int32_t inFileCount);
int32_t fs_dir_file_count(const char* inPath);

bool   fs_file_exists(const char *path);
bool   fs_file_is_absolute(const char *inPath);
size_t fs_file_size(const char *path);

const char* fs_exe_path(void);
const char* fs_exe_dir(void);
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void fs_create(int argc, char** argv);
void fs_cleanup(void);
//=============================================================================

#endif //FILESYSTEM_H
