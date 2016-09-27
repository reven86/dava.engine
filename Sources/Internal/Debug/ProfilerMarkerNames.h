#pragma once

namespace DAVA
{
class CPUMarkerName
{
public:
    //Core flow
    static const char* CORE_PROCESS_FRAME;
    static const char* CORE_BEGIN_FRAME;
    static const char* CORE_END_FRAME;
    static const char* CORE_JOB_MANAGER;
    static const char* CORE_APP_UPDATE;
    static const char* CORE_APP_DRAW;
    static const char* CORE_SOUND_SYSTEM;
    static const char* CORE_ANIMATION_MANAGER;
    static const char* CORE_UI_SYSTEM_UPDATE;
    static const char* CORE_UI_SYSTEM_DRAW;

    //Scene
    static const char* SCENE_UPDATE;
    static const char* SCENE_DRAW;
    static const char* SCENE_STATIC_OCCLUSION_SYSTEM;
    static const char* SCENE_ANIMATION_SYSTEM;
    static const char* SCENE_UPDATE_SYSTEM_PRE_TRANSFORM;
    static const char* SCENE_UPDATE_SYSTEM_POST_TRANSFORM;
    static const char* SCENE_TRANSFORM_SYSTEM;
    static const char* SCENE_LOD_SYSTEM;
    static const char* SCENE_SWITCH_SYSTEM;
    static const char* SCENE_PARTICLE_SYSTEM;
    static const char* SCENE_SOUND_UPDATE_SYSTEM;
    static const char* SCENE_RENDER_UPDATE_SYSTEM;
    static const char* SCENE_ACTION_UPDATE_SYSTEM;
    static const char* SCENE_DEBUG_RENDER_SYSTEM;
    static const char* SCENE_LANDSCAPE_SYSTEM;
    static const char* SCENE_FOLIAGE_SYSTEM;
    static const char* SCENE_SPEEDTREE_SYSTEM;
    static const char* SCENE_WIND_SYSTEM;
    static const char* SCENE_WAVE_SYSTEM;
    static const char* SCENE_SKELETON_SYSTEM;

    //Render
    static const char* RENDER_PASS_PREPARE_ARRAYS;
    static const char* RENDER_PASS_DRAW_LAYERS;
    static const char* RENDER_PREPARE_LANDSCAPE;

    //RHI
    static const char* RHI_RENDER_LOOP;
    static const char* RHI_PRESENT;
    static const char* RHI_DEVICE_PRESENT;
    static const char* RHI_EXECUTE_QUEUED_CMDS;
    static const char* RHI_EXECUTE_IMMEDIATE_CMDS;
    static const char* RHI_WAIT_IMMEDIATE_CMDS;
    static const char* RHI_WAIT_FRAME_EXECUTION;
    static const char* RHI_CMD_BUFFER_EXECUTE;
    static const char* RHI_WAIT_FRAME;
    static const char* RHI_PROCESS_SCHEDULED_DELETE;
};

class GPUMarkerName
{
public:
    static const char* GPU_FRAME;
    static const char* RENDER_PASS_2D;
    static const char* RENDER_PASS_MAIN_3D;
    static const char* RENDER_PASS_WATER_REFLECTION;
    static const char* RENDER_PASS_WATER_REFRACTION;
    static const char* LANDSCAPE;
};
}; //ns DAVA