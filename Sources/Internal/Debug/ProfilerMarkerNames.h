#pragma once

namespace DAVA
{
namespace ProfilerCPUMarkerName
{
//Core flow
extern const char* CORE_PROCESS_FRAME;
extern const char* CORE_BEGIN_FRAME;
extern const char* CORE_END_FRAME;
extern const char* CORE_JOB_MANAGER;
extern const char* CORE_APP_UPDATE;
extern const char* CORE_APP_DRAW;
extern const char* CORE_SOUND_SYSTEM;
extern const char* CORE_ANIMATION_MANAGER;
extern const char* CORE_UI_SYSTEM_UPDATE;
extern const char* CORE_UI_SYSTEM_DRAW;

//Scene
extern const char* SCENE_UPDATE;
extern const char* SCENE_DRAW;
extern const char* SCENE_STATIC_OCCLUSION_SYSTEM;
extern const char* SCENE_ANIMATION_SYSTEM;
extern const char* SCENE_UPDATE_SYSTEM_PRE_TRANSFORM;
extern const char* SCENE_UPDATE_SYSTEM_POST_TRANSFORM;
extern const char* SCENE_TRANSFORM_SYSTEM;
extern const char* SCENE_LOD_SYSTEM;
extern const char* SCENE_SWITCH_SYSTEM;
extern const char* SCENE_PARTICLE_SYSTEM;
extern const char* SCENE_SOUND_UPDATE_SYSTEM;
extern const char* SCENE_RENDER_UPDATE_SYSTEM;
extern const char* SCENE_ACTION_UPDATE_SYSTEM;
extern const char* SCENE_DEBUG_RENDER_SYSTEM;
extern const char* SCENE_LANDSCAPE_SYSTEM;
extern const char* SCENE_FOLIAGE_SYSTEM;
extern const char* SCENE_SPEEDTREE_SYSTEM;
extern const char* SCENE_WIND_SYSTEM;
extern const char* SCENE_WAVE_SYSTEM;
extern const char* SCENE_SKELETON_SYSTEM;

//Render
extern const char* RENDER_PASS_PREPARE_ARRAYS;
extern const char* RENDER_PASS_DRAW_LAYERS;
extern const char* RENDER_PREPARE_LANDSCAPE;

//RHI
extern const char* RHI_RENDER_LOOP;
extern const char* RHI_PRESENT;
extern const char* RHI_DEVICE_PRESENT;
extern const char* RHI_EXECUTE_QUEUED_CMDS;
extern const char* RHI_EXECUTE_IMMEDIATE_CMDS;
extern const char* RHI_WAIT_IMMEDIATE_CMDS;
extern const char* RHI_WAIT_FRAME_EXECUTION;
extern const char* RHI_CMD_BUFFER_EXECUTE;
extern const char* RHI_WAIT_FRAME;
extern const char* RHI_PROCESS_SCHEDULED_DELETE;
};

namespace ProfilerGPUMarkerName
{
extern const char* GPU_FRAME;
extern const char* RENDER_PASS_2D;
extern const char* RENDER_PASS_MAIN_3D;
extern const char* RENDER_PASS_WATER_REFLECTION;
extern const char* RENDER_PASS_WATER_REFRACTION;
extern const char* LANDSCAPE;
};
}; //ns DAVA