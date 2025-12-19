//===INCLUDES==================================================================
#include <kah_gfx/gfx_pool.h>

#include <kah_core/dynamic_array.h>
#include <kah_core/bit_array.h>
#include <kah_core/memory.h>
//=============================================================================

//===INTERNAL==================================================================
struct Pool{
    DynamicArray buffer;
    BitArrayDynamic freeEntries;
}typedef Pool;

static Pool pool_create(Allocator allocator, const uint32_t typeSize, const uint32_t startSize, const uint32_t poolSize){
    return (Pool){
        .buffer = dynamic_array_create(allocator, typeSize, startSize),
        .freeEntries = bitarray_dynamic_create(allocator, poolSize),
    };
}

static void pool_cleanup(Allocator allocator, Pool* pool){
    allocator.free(pool->buffer.info);
    allocator.free(pool->freeEntries.info);
    *pool = (Pool){};
}

static size_t pool_acquire_next_free_index(Pool* pool){
    size_t outIndex = bitarray_find_first_unset_bit(&pool->freeEntries.header);
    bitarray_set_bit(&pool->freeEntries.header, outIndex);

    if (outIndex >= pool->buffer.current) {
        const uint32_t resizeCount = ( pool->buffer.count + 1) * 1.5f;
        dynamic_array_resize( allocators()->cstd, &pool->buffer, resizeCount );
        pool->buffer.current = pool->buffer.count;
    }
    return outIndex;
}

static void pool_release_index(Pool* pool, uint32_t index){
    bitarray_clear_bit(&pool->freeEntries.header, index);
}
//=============================================================================

//===INTERNAL_STRUCTS==========================================================
static struct GfxPool{
    Pool gfxImages;
    Pool gfxTextures;
    Pool gfxMeshes;
    Pool transforms;
    Pool cameras;
    Pool cameraEntities;
    Pool litEntities;
    Pool litMaterials;
}s_pool = {};
//=============================================================================

//===POOL_IMPL_MACRO===========================================================

//<T_HANDLE>    gfx_pool_<NAME>_handle_get_next();
//<T>*          gfx_pool_<NAME>_get(<T_HANDLE> handle);
//void          gfx_pool_<NAME>_release(<T_HANDLE> handle);

#define GFX_POOL_IMPL(T, NAME, POOL_FIELD, POOL_COUNT_MAX)                              \
T##Handle gfx_pool_##NAME##_handle_get_next(void){                                      \
    T##Handle outHandle = (T##Handle)pool_acquire_next_free_index(&s_pool.POOL_FIELD);  \
    core_assert(outHandle < (POOL_COUNT_MAX));                                          \
    return outHandle;                                                                   \
}                                                                                       \
                                                                                        \
T* gfx_pool_##NAME##_get(T##Handle handle){                                             \
    core_assert(handle < (POOL_COUNT_MAX));                                             \
    return dynamic_array_get(&s_pool.POOL_FIELD.buffer, handle);                        \
}                                                                                       \
                                                                                        \
void gfx_pool_##NAME##_release(T##Handle handle){                                       \
    core_assert(handle < (POOL_COUNT_MAX));                                             \
    pool_release_index(&s_pool.POOL_FIELD, handle);                                     \
}
//=============================================================================

//===API=======================================================================
GFX_POOL_IMPL( GfxImage,    gfx_image,      gfxImages,      GFX_POOL_GFX_IMAGE_COUNT_MAX    )
GFX_POOL_IMPL( GfxTexture,  gfx_texture,    gfxTextures,    GFX_POOL_GFX_TEXTURE_COUNT_MAX  )
GFX_POOL_IMPL( GfxMesh,     gfx_mesh,       gfxMeshes,      GFX_POOL_GFX_MESH_COUNT_MAX     )
GFX_POOL_IMPL( Transform,   transform,      transforms,     GFX_POOL_TRANSFORM_COUNT_MAX    )
GFX_POOL_IMPL( Camera,      camera,         cameras,        GFX_POOL_CAMERA_COUNT_MAX       )
GFX_POOL_IMPL( CameraEntity,camera_entity,  cameraEntities, GFX_POOL_CAMERA_ENT_COUNT_MAX   )
GFX_POOL_IMPL( LitEntity,   lit_entity,     litEntities,    GFX_POOL_LIT_ENT_COUNT_MAX      )
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_pool_create(){
    s_pool.gfxImages =      pool_create(allocators()->cstd,  sizeof(GfxImage),       1, GFX_POOL_GFX_IMAGE_COUNT_MAX);
    s_pool.gfxTextures =    pool_create(allocators()->cstd,  sizeof(GfxTexture),     1, GFX_POOL_GFX_TEXTURE_COUNT_MAX);
    s_pool.gfxMeshes =      pool_create(allocators()->cstd,  sizeof(GfxMesh),        1, GFX_POOL_GFX_MESH_COUNT_MAX);
    s_pool.transforms =     pool_create(allocators()->cstd,  sizeof(Transform),      1, GFX_POOL_TRANSFORM_COUNT_MAX);
    s_pool.cameras =        pool_create(allocators()->cstd,  sizeof(Camera),         1, GFX_POOL_CAMERA_COUNT_MAX);
    s_pool.cameraEntities = pool_create(allocators()->cstd,  sizeof(CameraEntity),   1, GFX_POOL_CAMERA_ENT_COUNT_MAX);
    s_pool.litEntities =    pool_create(allocators()->cstd,  sizeof(LitEntity),      1, GFX_POOL_LIT_ENT_COUNT_MAX);
    s_pool.litMaterials =   pool_create(allocators()->cstd,  sizeof(LitMaterial),    1, GFX_POOL_LIT_MATERIAL_COUNT_MAX);
}

void gfx_pool_cleanup(){
    pool_cleanup(allocators()->cstd, &s_pool.gfxImages);
    pool_cleanup(allocators()->cstd, &s_pool.gfxTextures);
    pool_cleanup(allocators()->cstd, &s_pool.gfxMeshes);
    pool_cleanup(allocators()->cstd, &s_pool.transforms);
    pool_cleanup(allocators()->cstd, &s_pool.cameras);
    pool_cleanup(allocators()->cstd, &s_pool.cameraEntities);
    pool_cleanup(allocators()->cstd, &s_pool.litEntities);
    pool_cleanup(allocators()->cstd, &s_pool.litMaterials);
}
//=============================================================================
