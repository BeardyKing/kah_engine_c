//===INCLUDES==================================================================
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_IMPLEMENTATION
#define VMA_LEAK_LOG_FORMAT(format, ...)    \
{                                           \
printf((format), __VA_ARGS__);              \
printf("\n");                               \
}

#include <vk_mem_alloc.h>
//=============================================================================