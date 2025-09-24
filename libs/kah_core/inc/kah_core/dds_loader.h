#ifndef DDS_LOADER_H
#define DDS_LOADER_H

//===INCLUDES==================================================================
#include <kah_core/texture_formats.h>
//=============================================================================

//INFO: .dds file format spec
// https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-header
// https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-pixelformat
// https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
// https://learn.microsoft.com/en-us/windows/uwp/gaming/complete-code-for-ddstextureloader

//===API=======================================================================
void load_dds_image_alloc(const char *path, CoreRawImage *outRawImage);
//=============================================================================

#endif //DDS_LOADER_H
