#ifndef C_STRING_H
#define C_STRING_H

//===INCLUDES==================================================================
#include <kah_core/defines.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#ifdef _WIN32
#define strtok_r strtok_s
#endif
//=============================================================================

//===API=======================================================================
CORE_FORCE_INLINE bool c_str_empty(const char *inStr)                                   { return (inStr == NULL) || (inStr[0] == '\0'); }
CORE_FORCE_INLINE bool c_str_equal(const char *lhs, const char *rhs)                    { return (strcmp(lhs, rhs) == 0); }
CORE_FORCE_INLINE bool c_str_n_equal(const char *lhs, const char *rhs, size_t count)    { return (strncmp(lhs, rhs, count) == 0); }

char *c_str_search_reverse(char *src, const char *subStr);
char *c_str_n_search_reverse(char *src, int32_t srcLen, const char *subStr, int32_t subStrLen);

bool c_str_replace_after_delim_reverse(char *existingPath, const char *replaceTarget, const char *subStr);
bool c_str_replace_extension(char *existingPath, const char *newExtension);
bool c_str_remove_file_from_path(const char *inPath, char *outPath);
bool c_str_extract_file_name(char *inPath, char *outFilename);
//=============================================================================

#endif //C_STRING_H
