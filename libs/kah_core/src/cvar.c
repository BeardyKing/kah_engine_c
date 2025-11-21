
#include <stdio.h>
#include <stdlib.h>
#include <kah_core/cvar.h>
#include <kah_core/string_table.h>
#include <kah_core/allocators.h>
#include <kah_core/file_io.h>

#include "assert.h"
#include "c_string.h"
#include "filesystem.h"

constexpr size_t CVAR_INT32_ENTRY_COUNT = 128;

static StrTableInfo s_stringTable;
static AllocInfo* s_diskCVars = nullptr;

struct i32_cvarDesc { i32_cvar_t cvar; CVarLoadType loadType; } typedef i32_cvarDesc;
struct i32_cvarPair { char key[CVAR_NAME_SIZE]; struct i32_cvarDesc value; } typedef i32_cvarPair;

struct CVarTables {
    struct { i32_cvarPair table[CVAR_INT32_ENTRY_COUNT]; uint32_t count; } i32;

    uint32_t totalCount;
}typedef CVarTables;

static CVarTables s_cVarTables = (CVarTables){};

static i32_cvarPair* i32_cvar_get_entry(const char* name){
    i32_cvarPair* out = &s_cVarTables.i32.table[s_cVarTables.i32.count++];
#if KAH_DEBUG
    for (uint32_t i = 0; i < s_cVarTables.i32.count; ++i){
        core_assert(!c_str_equal(name, s_cVarTables.i32.table[i].key));
    }
#endif
    snprintf(out->key, CVAR_NAME_SIZE, "%s", name);
    return out;
}

static uint32_t search_str_table_for_cvar(const char* name){
    for (uint32_t i = 0; i < s_stringTable.rowCount; ++i){
        const char* dvarName = str_table_get_cell(&s_stringTable, i, 0);
        if(c_str_equal(dvarName, name)){
            return i;
        }
    }
    return UINT32_MAX;
}

i32_cvar_t *i32_cvar_create(const char* name, CVarLoadType loadType, int32_t defaultValue, int32_t min, int32_t max){
    //TODO: name length validation. based on CVAR_NAME_SIZE
    i32_cvarPair* tableEntry = i32_cvar_get_entry(name);
    tableEntry->value = (i32_cvarDesc){
        .cvar = (i32_cvar_t){ .current = defaultValue, .reset = defaultValue, .fallback = defaultValue, .min = min, .max = max },
        .loadType = loadType
    };

    if(loadType == C_VAR_DISK && s_diskCVars != nullptr){
        const uint32_t rowIndex = search_str_table_for_cvar(name);
        if(rowIndex != UINT32_MAX){
            //TODO: don't set this var or clamp if not within range.
            tableEntry->value.cvar.current = atoi(str_table_get_cell(&s_stringTable, rowIndex, 1));
            tableEntry->value.cvar.reset = tableEntry->value.cvar.current;
        }
    }

    return &tableEntry->value.cvar;
}

void cvar_create(const char* path){
    if(fs_file_exists(path)){
        s_diskCVars = file_io_load_into_buffer(allocators()->arena, path, true);
        str_table_parse_destructive(&s_stringTable, s_diskCVars->bufferAddress, ',');
    }
}

void cvar_cleanup(){
    //TODO: write file back to disk.
}