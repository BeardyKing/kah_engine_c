#ifndef STRING_TABLE_H
#define STRING_TABLE_H

//===INCLUDES==================================================================
#include <stdint.h>
//=============================================================================

//===TYPES=====================================================================
#define STR_TABLE_COL_MAX 64
#define STR_TABLE_ROW_MAX 64

struct StrTableCol {
    uint32_t count;
    const char* str[STR_TABLE_COL_MAX];
} typedef StrTableCol;

struct StrTableInfo{
    uint32_t rowCount;
    StrTableCol rows[STR_TABLE_ROW_MAX];
} typedef StrTableInfo;
//=============================================================================

//===API=======================================================================
void str_table_parse_destructive(StrTableInfo *out, char *inOutBuffer, char delim);
const char* str_table_get_cell(const StrTableInfo *inTable, uint32_t row, uint32_t col);
//=============================================================================

#endif //STRING_TABLE_H
