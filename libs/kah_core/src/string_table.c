//===INCLUDES==================================================================
#include <kah_core/string_table.h>
#include <kah_core/assert.h>

#include <string.h>
#include <stdio.h>
//=============================================================================

//===API=======================================================================
void str_table_parse_destructive(StrTableInfo *out, char *inOutBuffer, char delim){
    *out = (StrTableInfo){};

    char delimStr[2] = { delim, '\0' };
    char *saveLine = NULL;

    char *row = strtok_r(inOutBuffer, "\n", &saveLine);
    while (row && out->rowCount < STR_TABLE_ROW_MAX)
    {
        StrTableCol *strTableCol = &out->rows[out->rowCount];
        strTableCol->count = 0;

        char *saveField = nullptr;
        char *col = strtok_r(row, delimStr, &saveField);
        while (col && strTableCol->count < STR_TABLE_COL_MAX){
            strTableCol->str[strTableCol->count++] = col;
            col = strtok_r(nullptr, delimStr, &saveField);
        }

        out->rowCount++;
        row = strtok_r(nullptr, "\n", &saveLine);
    }
}

const char* str_table_get_cell(const StrTableInfo *inTable, uint32_t row, uint32_t col){
    core_assert(inTable != nullptr);
    core_assert(row < inTable->rowCount);
    core_assert(col < inTable->rows[row].count);

    const char *outStr = inTable->rows[row].str[col];
    core_assert(outStr != NULL);
    return outStr;
}


void str_table_print(const StrTableInfo *inTable){
    for (uint32_t r = 0; r < inTable->rowCount; ++r){
        for (uint32_t c = 0; c < inTable->rows[r].count; ++c){
            const char* str = str_table_get_cell(inTable,r,c);
            printf("| %s ", str);
        }
        printf("|\n");
    }
}
//=============================================================================