#ifndef FILE_READER_H
#define FILE_READER_H

//===INCLUDES==================================================================
#include <kah_core/allocators.h>
//=============================================================================

//===API=======================================================================
AllocInfo* file_io_load_into_buffer(Allocator allocator, const char *path, bool binaryFile);
void file_io_write_from_buffer(AllocInfo *allocInfo, const char *path, bool binary);
//=============================================================================

#endif //FILE_READER_H
