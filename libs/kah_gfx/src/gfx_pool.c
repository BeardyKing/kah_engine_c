//===INCLUDES==================================================================
#include <kah_gfx/gfx_pool.h>

#include <kah_core/dynamic_array.h>
#include <kah_core/fixed_array.h>
#include <kah_core/bit_array.h>
//=============================================================================

//===INTERNAL==================================================================
struct Pool{
    DynamicArray buffer;
    BitArrayDynamic freeEntries;
}typedef Pool;

static Pool pool_create(Allocator allocator, uint32_t typeSize, uint32_t count){
    return (Pool){
        .buffer = dynamic_array_create(allocator, typeSize, count),
        .freeEntries = bitarray_dynamic_create(allocator, count),
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
    return outIndex;
}

static void pool_release_index(Pool* pool, size_t index){
    GfxImage* image = dynamic_array_buffer(&pool->buffer);
    core_assert(image->image == VK_NULL_HANDLE);
    core_assert(image->view == VK_NULL_HANDLE);
    core_assert(image->alloc == VK_NULL_HANDLE);
    *image = (GfxImage){};
    bitarray_clear_bit(&pool->freeEntries.header, index);
}
//=============================================================================

//===INTERNAL_STRUCTS==========================================================
static struct GfxPool{
    Pool gfxImages;
}s_pool = {};
//=============================================================================

//===API=======================================================================
GfxImageHandle gfx_pool_get_gfx_image_handle(){
    GfxImageHandle outHandle = truncate_cast(GfxImageHandle, pool_acquire_next_free_index(&s_pool.gfxImages));
    core_assert(outHandle < GFX_POOL_GFX_IMAGE_COUNT_MAX);
    return outHandle;
}

GfxImage* gfx_pool_get_gfx_image(GfxImageHandle handle){
    core_assert(handle < GFX_POOL_GFX_IMAGE_COUNT_MAX);
    return dynamic_array_get(&s_pool.gfxImages.buffer, handle);
}

void gfx_pool_release_gfx_image(GfxImageHandle handle){
    pool_release_index(&s_pool.gfxImages,handle);
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_pool_create(){
    s_pool.gfxImages = pool_create(allocators()->page, sizeof(GfxImage), GFX_POOL_GFX_IMAGE_COUNT_MAX);
}

void gfx_pool_cleanup(){
    pool_cleanup(allocators()->page, &s_pool.gfxImages);
}
//=============================================================================
