//===INCLUDES==================================================================
#include <kah_core/file_io.h>
#include <kah_core/allocators.h>
#include <kah_core/assert.h>
#include <kah_core/filesystem.h>

#include <stdio.h>
//=============================================================================

//===API=======================================================================
AllocInfo* file_io_load_into_buffer(Allocator allocator, const char *path, bool binaryFile)
{
    core_assert_msg(path != NULL, "err: File path is nullptr");
    core_assert_msg(allocator.alloc, "err: Allocator not valid");
    core_assert_msg(fs_file_exists(path), "err: File does not exist");

    const size_t filesize = fs_file_size(path);
    core_assert_msg(filesize > 0, "err: File is empty");

    const size_t allocSize = binaryFile ? filesize : filesize + 1;
    AllocInfo *allocInfo = allocator.alloc(allocSize);
    core_assert_msg(allocInfo && allocInfo->bufferAddress, "err: Allocator failed");

    FILE *fp = fopen(path, binaryFile ? "rb" : "r");
    {
        core_assert_msg(fp != NULL, "err: Failed to open file");
        size_t bytesRead = fread(allocInfo->bufferAddress, 1, filesize, fp);
        core_assert_msg(bytesRead == filesize, "err: Failed to read full file");
    }
    fclose(fp);

    if (!binaryFile){
        ((char*)allocInfo->bufferAddress)[filesize] = '\0';
    }

    return allocInfo;
}

void file_io_write_from_buffer(AllocInfo *allocInfo, const char *path, bool binary){
    core_assert_msg(path != NULL, "err: File path is nullptr");
    core_assert_msg(allocInfo != NULL && allocInfo->bufferAddress, "err: AllocInfo invalid");

    FILE *fp = fopen(path, binary ? "wb" : "w");
    {
        core_assert_msg(fp != NULL, "err: Failed to open file for writing");
        const size_t bytesWritten = fwrite(allocInfo->bufferAddress, 1, allocInfo->commitedMemory, fp);
        core_assert_msg(bytesWritten == allocInfo->commitedMemory, "err: Failed to write full file");
    }
    fclose(fp);
}
//=============================================================================
