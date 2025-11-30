//===INCLUDES==================================================================
#include <kah_core/cvar.h>
#include <kah_core/string_table.h>
#include <kah_core/allocators.h>
#include <kah_core/file_io.h>
#include <kah_core/assert.h>
#include <kah_core/c_string.h>
#include <kah_core/filesystem.h>

#include <kah_math/utils.h>

#include <stdio.h>
#include <stdlib.h>
//=============================================================================

//===STRUCTS_INTERNAL==========================================================
static StrTableInfo s_stringTable;
static AllocInfo* s_diskCVars = nullptr;

struct bool_cvarDesc { bool_cvar_t cvar; CVarLoadType loadType; } typedef bool_cvarDesc;
struct bool_cvarPair { char key[CVAR_NAME_SIZE + 1]; struct bool_cvarDesc value; } typedef bool_cvarPair; //TODO: replace key with cvar name hash.

struct i32_cvarDesc { i32_cvar_t cvar; CVarLoadType loadType; } typedef i32_cvarDesc;
struct i32_cvarPair { char key[CVAR_NAME_SIZE + 1]; struct i32_cvarDesc value; } typedef i32_cvarPair; //TODO: replace key with cvar name hash.

struct u32_cvarDesc { u32_cvar_t cvar; CVarLoadType loadType; } typedef u32_cvarDesc;
struct u32_cvarPair { char key[CVAR_NAME_SIZE + 1]; struct u32_cvarDesc value; } typedef u32_cvarPair; //TODO: replace key with cvar name hash.

struct f32_cvarDesc { f32_cvar_t cvar; CVarLoadType loadType; } typedef f32_cvarDesc;
struct f32_cvarPair { char key[CVAR_NAME_SIZE + 1]; struct f32_cvarDesc value; } typedef f32_cvarPair; //TODO: replace key with cvar name hash.

struct vec2i_cvarDesc { vec2i_cvar_t cvar; CVarLoadType loadType; } typedef vec2i_cvarDesc;
struct vec2i_cvarPair { char key[CVAR_NAME_SIZE + 1]; struct vec2i_cvarDesc value; } typedef vec2i_cvarPair; //TODO: replace key with cvar name hash.

constexpr size_t CVAR_BOOL8_ENTRY_COUNT = 32;
constexpr size_t CVAR_INT32_ENTRY_COUNT = 32;
constexpr size_t CVAR_UINT32_ENTRY_COUNT = 32;
constexpr size_t CVAR_FLOAT32_ENTRY_COUNT = 32;
constexpr size_t CVAR_VEC2I32_ENTRY_COUNT = 32;

struct CVarTables {
    struct { bool_cvarPair   table[CVAR_BOOL8_ENTRY_COUNT];   uint32_t count; } b8;
    struct { i32_cvarPair    table[CVAR_INT32_ENTRY_COUNT];   uint32_t count; } i32;
    struct { u32_cvarPair    table[CVAR_UINT32_ENTRY_COUNT];  uint32_t count; } u32;
    struct { f32_cvarPair    table[CVAR_FLOAT32_ENTRY_COUNT]; uint32_t count; } f32;
    struct { vec2i_cvarPair  table[CVAR_VEC2I32_ENTRY_COUNT]; uint32_t count; } vec2i;
}typedef CVarTables;
//=============================================================================

//===DATA_INTERNAL=============================================================
static CVarTables s_cVarTables = (CVarTables){};
static char s_cvarPath[KAH_FILESYSTEM_MAX_PATH] = {};
//=============================================================================

//===INTERNAL==================================================================

static bool_cvarPair* bool_cvar_get_entry(const char* name) {
    bool_cvarPair* out = &s_cVarTables.b8.table[s_cVarTables.b8.count++];
#if KAH_DEBUG
    for (uint32_t i = 0; i < s_cVarTables.b8.count; ++i) {
        core_assert(!c_str_equal(name, s_cVarTables.b8.table[i].key));
    }
#endif
    snprintf(out->key, CVAR_NAME_SIZE, "%s", name);
    return out;
}

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

static u32_cvarPair* u32_cvar_get_entry(const char* name) {
    u32_cvarPair* out = &s_cVarTables.u32.table[s_cVarTables.u32.count++];
#if KAH_DEBUG
    for (uint32_t i = 0; i < s_cVarTables.u32.count; ++i) {
        core_assert(!c_str_equal(name, s_cVarTables.u32.table[i].key));
    }
#endif
    snprintf(out->key, CVAR_NAME_SIZE, "%s", name);
    return out;
}

static f32_cvarPair* f32_cvar_get_entry(const char* name) {
    f32_cvarPair* out = &s_cVarTables.f32.table[s_cVarTables.f32.count++];
#if KAH_DEBUG
    for (uint32_t i = 0; i < s_cVarTables.f32.count; ++i) {
        core_assert(!c_str_equal(name, s_cVarTables.f32.table[i].key));
    }
#endif
    snprintf(out->key, CVAR_NAME_SIZE, "%s", name);
    return out;
}

static vec2i_cvarPair* vec2i_cvar_get_entry(const char* name) {
    vec2i_cvarPair* out = &s_cVarTables.vec2i.table[s_cVarTables.vec2i.count++];
#if KAH_DEBUG
    for (uint32_t i = 0; i < s_cVarTables.vec2i.count; ++i) {
        core_assert(!c_str_equal(name, s_cVarTables.vec2i.table[i].key));
    }
#endif
    snprintf(out->key, CVAR_NAME_SIZE, "%s", name);
    return out;
}


static uint32_t search_str_table_for_cvar(const char* name){
    for (uint32_t i = 0; i < s_stringTable.rowCount; ++i){
        const char* cvarName = str_table_get_cell(&s_stringTable, i, 0);
        if(c_str_equal(cvarName, name)){
            return i;
        }
    }
    return UINT32_MAX;
}
//=============================================================================

//===INTERNAL_SERIALISE========================================================
static void cvar_serialise_disk_vars_bool(FILE* fp) {
    for (uint32_t i = 0; i < s_cVarTables.b8.count; i++) {
        bool_cvarPair* cvarEntry = &s_cVarTables.b8.table[i];
        if (cvarEntry->value.loadType == C_VAR_DISK) {
            fprintf(fp, "%s,%i\n", s_cVarTables.b8.table[i].key, !!cvarEntry->value.cvar.current);
        }
    }
}

static void cvar_serialise_disk_vars_i32(FILE* fp) {
    for (uint32_t i = 0; i < s_cVarTables.i32.count; i++) {
        i32_cvarPair* cvarEntry = &s_cVarTables.i32.table[i];
        if (cvarEntry->value.loadType == C_VAR_DISK) {
            fprintf(fp, "%s,%i\n", s_cVarTables.i32.table[i].key, cvarEntry->value.cvar.current);
        }
    }
}

static void cvar_serialise_disk_vars_u32(FILE* fp) {
    for (uint32_t i = 0; i < s_cVarTables.u32.count; i++) {
        u32_cvarPair* cvarEntry = &s_cVarTables.u32.table[i];
        if (cvarEntry->value.loadType == C_VAR_DISK) {
            fprintf(fp, "%s,%u\n", s_cVarTables.u32.table[i].key, cvarEntry->value.cvar.current);
        }
    }
}

static void cvar_serialise_disk_vars_f32(FILE* fp) {
    for (uint32_t i = 0; i < s_cVarTables.f32.count; i++) {
        f32_cvarPair* cvarEntry = &s_cVarTables.f32.table[i];
        if (cvarEntry->value.loadType == C_VAR_DISK) {
            fprintf(fp, "%s,%.5f\n", s_cVarTables.f32.table[i].key, cvarEntry->value.cvar.current);
        }
    }
}

static void cvar_serialise_disk_vars_vec2i(FILE* fp) {
    for (uint32_t i = 0; i < s_cVarTables.vec2i.count; i++) {
        vec2i_cvarPair* cvarEntry = &s_cVarTables.vec2i.table[i];
        if (cvarEntry->value.loadType == C_VAR_DISK) {
            fprintf(fp, "%s,%i,%i\n", s_cVarTables.vec2i.table[i].key, cvarEntry->value.cvar.current.x, cvarEntry->value.cvar.current.y);
        }
    }
}
//=============================================================================

//===API=======================================================================
void cvar_serialise_disk_vars() {
    core_assert(!c_str_empty(s_cvarPath));
    FILE* fp = fopen(s_cvarPath, "w");{
        core_assert(fp);
        cvar_serialise_disk_vars_bool(fp);
        cvar_serialise_disk_vars_i32(fp);
        cvar_serialise_disk_vars_u32(fp);
        cvar_serialise_disk_vars_f32(fp);
        cvar_serialise_disk_vars_vec2i(fp);
    }
    fclose(fp);
}

bool_cvar_t* bool_cvar_create(const char* name, CVarLoadType loadType, bool defaultValue) {
    core_assert(strlen(name) < (sizeof(char) * CVAR_NAME_SIZE));
    bool_cvarPair* tableEntry = bool_cvar_get_entry(name);
    tableEntry->value = (bool_cvarDesc){
        .cvar = (bool_cvar_t){.current = defaultValue, .reset = defaultValue, .fallback = defaultValue},
        .loadType = loadType
    };

    if (loadType == C_VAR_DISK && s_diskCVars != nullptr) {
        const uint32_t rowIndex = search_str_table_for_cvar(name); //TODO: replace with hash lookup.
        if (rowIndex != UINT32_MAX) {
            const bool current = !!atoi(str_table_get_cell(&s_stringTable, rowIndex, 1));
            tableEntry->value.cvar.current = current;
            tableEntry->value.cvar.reset = tableEntry->value.cvar.current;
        }
    }
    return &tableEntry->value.cvar;
}

i32_cvar_t *i32_cvar_create(const char* name, CVarLoadType loadType, int32_t defaultValue, int32_t min, int32_t max){
    core_assert(strlen(name) < (sizeof(char) * CVAR_NAME_SIZE));
    i32_cvarPair* tableEntry = i32_cvar_get_entry(name);
    tableEntry->value = (i32_cvarDesc){
        .cvar = (i32_cvar_t){ .current = defaultValue, .reset = defaultValue, .fallback = defaultValue, .min = min, .max = max },
        .loadType = loadType
    };

    if(loadType == C_VAR_DISK && s_diskCVars != nullptr){
        const uint32_t rowIndex = search_str_table_for_cvar(name); //TODO: replace with hash lookup.
        if(rowIndex != UINT32_MAX){
            const int32_t current = clamp_i32(atoi(str_table_get_cell(&s_stringTable, rowIndex, 1)), min, max);
            tableEntry->value.cvar.current = current;
            tableEntry->value.cvar.reset = tableEntry->value.cvar.current;
        }
    }
    return &tableEntry->value.cvar;
}

u32_cvar_t* u32_cvar_create(const char* name, CVarLoadType loadType, uint32_t defaultValue, uint32_t min, uint32_t max) {
    core_assert(strlen(name) < (sizeof(char) * CVAR_NAME_SIZE));
    u32_cvarPair* tableEntry = u32_cvar_get_entry(name);
    tableEntry->value = (u32_cvarDesc){
        .cvar = (u32_cvar_t){.current = defaultValue, .reset = defaultValue, .fallback = defaultValue, .min = min, .max = max },
        .loadType = loadType
    };

    if (loadType == C_VAR_DISK && s_diskCVars != nullptr) {
        const uint32_t rowIndex = search_str_table_for_cvar(name); //TODO: replace with hash lookup.
        if (rowIndex != UINT32_MAX) {
            const uint32_t current = clamp_u32(strtoul(str_table_get_cell(&s_stringTable, rowIndex, 1), NULL, 10), min, max);
            tableEntry->value.cvar.current = current;
            tableEntry->value.cvar.reset = tableEntry->value.cvar.current;
        }
    }
    return &tableEntry->value.cvar;
}


f32_cvar_t* f32_cvar_create(const char* name, CVarLoadType loadType, float defaultValue, float min, float max) {
    core_assert(strlen(name) < (sizeof(char) * CVAR_NAME_SIZE));
    f32_cvarPair* tableEntry = f32_cvar_get_entry(name);
    tableEntry->value = (f32_cvarDesc){
        .cvar = (f32_cvar_t){.current = defaultValue, .reset = defaultValue, .fallback = defaultValue, .min = min, .max = max },
        .loadType = loadType
    };

    if (loadType == C_VAR_DISK && s_diskCVars != nullptr) {
        const uint32_t rowIndex = search_str_table_for_cvar(name); //TODO: replace with hash lookup.
        if (rowIndex != UINT32_MAX) {
            const float current = clamp_f32(strtof(str_table_get_cell(&s_stringTable, rowIndex, 1), NULL), min, max);
            tableEntry->value.cvar.current = current;
            tableEntry->value.cvar.reset = tableEntry->value.cvar.current;
        }
    }
    return &tableEntry->value.cvar;
}

vec2i_cvar_t* vec2i_cvar_create(const char* name, CVarLoadType loadType, vec2i defaultValue, vec2i min, vec2i max) {
    core_assert(strlen(name) < (sizeof(char) * CVAR_NAME_SIZE));
    vec2i_cvarPair* tableEntry = vec2i_cvar_get_entry(name);
    tableEntry->value = (vec2i_cvarDesc){
        .cvar = (vec2i_cvar_t){.current = defaultValue, .reset = defaultValue, .fallback = defaultValue, .min = min, .max = max },
        .loadType = loadType
    };

    if (loadType == C_VAR_DISK && s_diskCVars != nullptr) {
        const uint32_t rowIndex = search_str_table_for_cvar(name); //TODO: replace with hash lookup.
        if (rowIndex != UINT32_MAX) {
            const int32_t currentX = clamp_i32(atoi(str_table_get_cell(&s_stringTable, rowIndex, 1)), min.x, max.x);
            const int32_t currentY = clamp_i32(atoi(str_table_get_cell(&s_stringTable, rowIndex, 2)), min.y, max.y);
            tableEntry->value.cvar.current = (vec2i){ .x = currentX, .y = currentY };
            tableEntry->value.cvar.reset = tableEntry->value.cvar.current;
        }
    }
    return &tableEntry->value.cvar;
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void cvar_create(const char* path){
    core_assert(strlen(path) < KAH_FILESYSTEM_MAX_PATH);
    strcpy(s_cvarPath, path);
    if(fs_file_exists(path) && fs_file_size(path)){
        s_diskCVars = file_io_load_into_buffer(allocators()->arena, path, true);
        str_table_parse_destructive(&s_stringTable, s_diskCVars->bufferAddress, ',');
    }
}

void cvar_cleanup(){
    cvar_serialise_disk_vars();
}
//=============================================================================