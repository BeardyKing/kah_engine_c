#include <kah_gfx/vulkan/gfx_vulkan_bindless.h>
#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>

#include <kah_core/assert.h>

//===EXTERNAL_STRUCTS==========================================================
extern GlobalGfx g_gfx;
//=============================================================================

//===INTERNAL_DEFINES==========================================================
static constexpr uint32_t KAH_BINDLESS_POOL_SIZE = 2;

static constexpr uint32_t KAH_ANISOTROPY_MAX = 16;
static constexpr uint32_t KAH_MIP_MAP_MAX = 16;

static constexpr uint32_t KAH_BINDLESS_TEXTURE_MAX = 16536;
static constexpr uint32_t KAH_BINDLESS_SAMPLERS_MAX = 32;

static constexpr uint32_t KAH_DESCRIPTOR_SET_BINDING_TEXTURE = 0;
static constexpr uint32_t KAH_DESCRIPTOR_SET_BINDING_SAMPLER = 1;
//=============================================================================

//===INTERNAL_STRUCTS==========================================================
static struct GfxBindless{
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;

    struct{
        VkSampler nearest;
        VkSampler linear;
    }samplers;
} s_gfxBindless;
//=============================================================================

//===INTERNAL_FUNCTIONS========================================================
static void gfx_samplers_create(){
    {
        const VkSamplerCreateInfo info = (VkSamplerCreateInfo){
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_NEAREST,
            .minFilter = VK_FILTER_NEAREST,
        };
        const VkResult res = vkCreateSampler(g_gfx.device, &info, g_gfx.allocationCallbacks, &s_gfxBindless.samplers.nearest);
        core_assert(res == VK_SUCCESS);
    }
    {
        const VkSamplerCreateInfo info = (VkSamplerCreateInfo){
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .minLod = 0.0f,
            .maxLod = KAH_MIP_MAP_MAX,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = KAH_ANISOTROPY_MAX,
        };
        const VkResult res = vkCreateSampler(g_gfx.device, &info, g_gfx.allocationCallbacks, &s_gfxBindless.samplers.linear);
        core_assert(res == VK_SUCCESS);
    }
    gfx_bindless_set_sampler(KAH_SAMPLER_ID_NEAREST, s_gfxBindless.samplers.nearest);
    gfx_bindless_set_sampler(KAH_SAMPLER_ID_LINEAR, s_gfxBindless.samplers.linear);
}

static void gfx_sampler_cleanup(){
    vkDestroySampler(g_gfx.device, s_gfxBindless.samplers.nearest, g_gfx.allocationCallbacks);
    vkDestroySampler(g_gfx.device, s_gfxBindless.samplers.linear, g_gfx.allocationCallbacks);
}

static void gfx_bindless_descriptor_create(){
    {
         VkDescriptorPoolSize poolSizesBindless[KAH_BINDLESS_POOL_SIZE] = {
            (VkDescriptorPoolSize){VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, KAH_BINDLESS_TEXTURE_MAX},
            (VkDescriptorPoolSize){VK_DESCRIPTOR_TYPE_SAMPLER, KAH_BINDLESS_SAMPLERS_MAX},
        };

        const VkDescriptorPoolCreateInfo poolInfo = (VkDescriptorPoolCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
            .maxSets = KAH_BINDLESS_TEXTURE_MAX * KAH_BINDLESS_POOL_SIZE,
            .poolSizeCount = KAH_BINDLESS_POOL_SIZE,
            .pPoolSizes = &poolSizesBindless[0],
        };

        const VkResult result = vkCreateDescriptorPool(g_gfx.device, &poolInfo, g_gfx.allocationCallbacks, &s_gfxBindless.descriptorPool);
        core_assert(result == VK_SUCCESS);
    }
    {
        const VkDescriptorSetLayoutBinding bindings[KAH_BINDLESS_POOL_SIZE] = {
            (VkDescriptorSetLayoutBinding){
                .binding = KAH_DESCRIPTOR_SET_BINDING_TEXTURE,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ,
                .descriptorCount = KAH_BINDLESS_TEXTURE_MAX,
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = nullptr,
            },
            (VkDescriptorSetLayoutBinding){
            .binding = KAH_DESCRIPTOR_SET_BINDING_SAMPLER,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = KAH_BINDLESS_SAMPLERS_MAX ,
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr
            }
        };

        VkDescriptorBindingFlags bindingFlags[KAH_BINDLESS_POOL_SIZE] = {
            (VkDescriptorBindingFlags){VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
            (VkDescriptorBindingFlags){VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT}
        };

        const VkDescriptorSetLayoutBindingFlagsCreateInfo flagInfo = (VkDescriptorSetLayoutBindingFlagsCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = KAH_BINDLESS_POOL_SIZE,
            .pBindingFlags = &bindingFlags[0],
        };

        const VkDescriptorSetLayoutCreateInfo info = (VkDescriptorSetLayoutCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &flagInfo,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
            .bindingCount = KAH_BINDLESS_POOL_SIZE,
            .pBindings = &bindings[0],
        };

        const VkResult res = vkCreateDescriptorSetLayout(g_gfx.device, &info, nullptr, &s_gfxBindless.descriptorSetLayout);
        core_assert( res == VK_SUCCESS );
    }
    {
        constexpr uint32_t KAH_MAX_BINDINGS = KAH_BINDLESS_POOL_SIZE - 1;
        const VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo = (VkDescriptorSetVariableDescriptorCountAllocateInfo){
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
            .descriptorSetCount = 1,
            .pDescriptorCounts = &KAH_MAX_BINDINGS,
        };

        const VkDescriptorSetAllocateInfo allocInfo = (VkDescriptorSetAllocateInfo){
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = s_gfxBindless.descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &s_gfxBindless.descriptorSetLayout,
            .pNext = &countInfo,
        };

        const VkResult res = vkAllocateDescriptorSets(g_gfx.device, &allocInfo, &s_gfxBindless.descriptorSet);
        core_assert(res == VK_SUCCESS);
    }
}

static void gfx_bindless_descriptor_cleanup(){
    vkDestroyDescriptorSetLayout(g_gfx.device, s_gfxBindless.descriptorSetLayout, g_gfx.allocationCallbacks);
    vkDestroyDescriptorPool(g_gfx.device, s_gfxBindless.descriptorPool, g_gfx.allocationCallbacks);
}
//=============================================================================

//===API=======================================================================
void gfx_bindless_set_sampler(uint32_t samplerIndex, VkSampler sampler){
    const VkDescriptorImageInfo info = (VkDescriptorImageInfo){.sampler = sampler, .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL};
    const VkWriteDescriptorSet writeSet = (VkWriteDescriptorSet){
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = s_gfxBindless.descriptorSet,
        .dstBinding = KAH_DESCRIPTOR_SET_BINDING_SAMPLER,
        .dstArrayElement = samplerIndex,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .pImageInfo = &info,
    };
    vkUpdateDescriptorSets(g_gfx.device, 1, &writeSet, 0, nullptr);
}

void gfx_bindless_set_image(uint32_t imageIndex, VkImageView imageView){
    const VkDescriptorImageInfo info = (VkDescriptorImageInfo){ .imageView = imageView, .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL};
    const VkWriteDescriptorSet writeSet = (VkWriteDescriptorSet){
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = s_gfxBindless.descriptorSet,
        .dstBinding = KAH_DESCRIPTOR_SET_BINDING_TEXTURE,
        .dstArrayElement = imageIndex,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = &info,
    };
    vkUpdateDescriptorSets(g_gfx.device, 1, &writeSet, 0, nullptr);
}

VkSampler gfx_get_sampler_linear(){
    return s_gfxBindless.samplers.linear;
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_bindless_create(){
    gfx_bindless_descriptor_create();
    gfx_samplers_create();
}

void gfx_bindless_cleanup(){
    gfx_sampler_cleanup();
    gfx_bindless_descriptor_cleanup();
}
//=============================================================================
