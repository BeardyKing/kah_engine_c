#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//===DEFINES===================================================================
#define KAH_FILESYSTEM_MAX_PATH 260 //MAX_PATH
//===API=======================================================================

//=============================================================================
bool fs_mkdir(const char *path);
bool fs_mkdir_recursive(const char *path);

bool fs_rmdir(const char *path);

bool fs_file_exists(const char *path);
bool fs_file_is_absolute(const char *inPath);

size_t fs_file_size(const char *path);
//=============================================================================


#endif //FILESYSTEM_H
