#ifndef GFX_TASK_GRAPH_H
#define GFX_TASK_GRAPH_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h> // TODO: Make task graph backend agnostic.

#include <stdbool.h>
//=============================================================================

//===API=======================================================================
void gfx_task_graph_run(VkCommandBuffer cmdBuffer);
void gfx_task_graph_build();
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_task_graph_create(bool fullCreate);
void gfx_task_graph_cleanup(bool fullCleanup);
//=============================================================================

#endif //GFX_TASK_GRAPH_H
