//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan_uniform_buffers.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>
#include <kah_gfx/gfx_types.h>

#include <kah_core/assert.h>
//=============================================================================

//===EXTERNAL_STRUCTS==========================================================
extern GlobalGfx g_gfx;
//=============================================================================

//===INTERNAL_STRUCTS==========================================================
static struct UniformBufferObjects{
    struct{
        GfxBuffer gfxBuffer;
        VkDescriptorPool descriptorPool;
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSet descriptorSet;
    }scene;
} s_uniformBufferObjects = {};
//=============================================================================

//===API=======================================================================
GfxBuffer* gfx_uniform_buffer_scene_get(){
    core_assert(s_uniformBufferObjects.scene.gfxBuffer.buffer != VK_NULL_HANDLE);
    return &s_uniformBufferObjects.scene.gfxBuffer;
}

VkDescriptorSet gfx_uniform_buffer_scene_descriptor_set(){
    return s_uniformBufferObjects.scene.descriptorSet;
}

VkDescriptorSetLayout gfx_uniform_buffer_scene_descriptor_set_layout(){
    core_assert(s_uniformBufferObjects.scene.descriptorSetLayout != VK_NULL_HANDLE);
    return s_uniformBufferObjects.scene.descriptorSetLayout;
}

void gfx_uniform_buffer_scene_create(){
    s_uniformBufferObjects.scene.gfxBuffer = gfx_buffer_create(
        sizeof(SceneUBO),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO
    );

    {
        VkDescriptorPoolSize descriptorPoolSizes[1] = {
            (VkDescriptorPoolSize){VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
        };

        const VkDescriptorPoolCreateInfo poolInfo = (VkDescriptorPoolCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
            .maxSets = 1,
            .poolSizeCount = 1,
            .pPoolSizes = &descriptorPoolSizes[0],
        };
        const VkResult result = vkCreateDescriptorPool(g_gfx.device, &poolInfo, g_gfx.allocationCallbacks, &s_uniformBufferObjects.scene.descriptorPool);
        core_assert(result == VK_SUCCESS);
    }
    {
        const VkDescriptorSetLayoutBinding bindings[1] = {
            (VkDescriptorSetLayoutBinding){
                .binding = KAH_DESCRIPTOR_SET_BINDING_UBO_SCENE,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = nullptr,
            }
        };

        VkDescriptorBindingFlags bindingFlags = { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT};

        const VkDescriptorSetLayoutBindingFlagsCreateInfo flagInfo = (VkDescriptorSetLayoutBindingFlagsCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = 1,
            .pBindingFlags = &bindingFlags,
        };

        const VkDescriptorSetLayoutCreateInfo info = (VkDescriptorSetLayoutCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &flagInfo,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
            .bindingCount = 1,
            .pBindings = &bindings[0],
        };

        const VkResult res = vkCreateDescriptorSetLayout(g_gfx.device, &info, nullptr, &s_uniformBufferObjects.scene.descriptorSetLayout);
        core_assert( res == VK_SUCCESS );
    }
    {
        const VkDescriptorSetAllocateInfo allocInfo = (VkDescriptorSetAllocateInfo){
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = s_uniformBufferObjects.scene.descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &s_uniformBufferObjects.scene.descriptorSetLayout,
            .pNext = nullptr,
        };

        const VkResult res = vkAllocateDescriptorSets(g_gfx.device, &allocInfo, &s_uniformBufferObjects.scene.descriptorSet);
        core_assert(res == VK_SUCCESS);

        VkDescriptorBufferInfo descBufferInfo = (VkDescriptorBufferInfo){
            .buffer = s_uniformBufferObjects.scene.gfxBuffer.buffer,
            .offset = 0,
            .range  = sizeof(SceneUBO),
        };

        VkWriteDescriptorSet writeDescSet = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = s_uniformBufferObjects.scene.descriptorSet,
            .dstBinding = KAH_DESCRIPTOR_SET_BINDING_UBO_SCENE,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo = &descBufferInfo,
            .pImageInfo = nullptr,
            .pTexelBufferView = nullptr,
        };

        vkUpdateDescriptorSets(g_gfx.device, 1, &writeDescSet, 0, nullptr);
    }
}


void gfx_uniform_buffer_scene_cleanup(){
    gfx_buffer_cleanup(&s_uniformBufferObjects.scene.gfxBuffer);
    vkDestroyDescriptorSetLayout(g_gfx.device, s_uniformBufferObjects.scene.descriptorSetLayout, g_gfx.allocationCallbacks);
    vkDestroyDescriptorPool(g_gfx.device, s_uniformBufferObjects.scene.descriptorPool,g_gfx.allocationCallbacks );
}

//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_uniform_buffers_create(){
    gfx_uniform_buffer_scene_create();
}

void gfx_uniform_buffers_cleanup(){
    gfx_uniform_buffer_scene_cleanup();
}
//=============================================================================
