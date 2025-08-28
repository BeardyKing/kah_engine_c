#ifndef GFX_TASK_GRAPH_H
#define GFX_TASK_GRAPH_H

//===INCLUDES=======================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h> // TODO: Make task graph backend agnostic.
//=============================================================================

//===API=======================================================================
void gfx_task_graph_run(VkCommandBuffer cmdBuffer);
void gfx_task_graph_build();
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_task_graph_create();
void gfx_task_graph_cleanup();
//=============================================================================

#endif //GFX_TASK_GRAPH_H
