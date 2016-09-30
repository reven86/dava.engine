#pragma once

namespace DAVA
{
namespace CPUMarkerName
{
//Core flow
static const char* CORE_PROCESS_FRAME = "Core::SystemProcessFrame";
static const char* CORE_BEGIN_FRAME = "Core::BeginFrame";
static const char* CORE_END_FRAME = "Core::EndFrame";
static const char* CORE_JOB_MANAGER = "JobManager";
static const char* CORE_APP_UPDATE = "ApplicationCore::Update";
static const char* CORE_APP_DRAW = "ApplicationCore::Draw";
static const char* CORE_SOUND_SYSTEM = "SoundSystem";
static const char* CORE_ANIMATION_MANAGER = "AnimationManager";
static const char* CORE_UI_SYSTEM_UPDATE = "UI::Update";
static const char* CORE_UI_SYSTEM_DRAW = "UI::Draw";

//Scene
static const char* SCENE_UPDATE = "Scene::Update";
static const char* SCENE_DRAW = "Scene::Draw";
static const char* SCENE_STATIC_OCCLUSION_SYSTEM = "StaticOcclusionSystem";
static const char* SCENE_ANIMATION_SYSTEM = "AnimationSystem";
static const char* SCENE_UPDATE_SYSTEM_PRE_TRANSFORM = "UpdateSystem::PreTransform";
static const char* SCENE_UPDATE_SYSTEM_POST_TRANSFORM = "UpdateSystem::PostTransform";
static const char* SCENE_TRANSFORM_SYSTEM = "TransformSystem";
static const char* SCENE_LOD_SYSTEM = "LodSystem";
static const char* SCENE_SWITCH_SYSTEM = "SwitchSystem";
static const char* SCENE_PARTICLE_SYSTEM = "ParticleEffectSystem";
static const char* SCENE_SOUND_UPDATE_SYSTEM = "SoundUpdateSystem";
static const char* SCENE_RENDER_UPDATE_SYSTEM = "RenderUpdateSystem";
static const char* SCENE_ACTION_UPDATE_SYSTEM = "ActionUpdateSystem";
static const char* SCENE_DEBUG_RENDER_SYSTEM = "DebugRenderSystem";
static const char* SCENE_LANDSCAPE_SYSTEM = "LandscapeSystem";
static const char* SCENE_FOLIAGE_SYSTEM = "FoliageSystem";
static const char* SCENE_SPEEDTREE_SYSTEM = "SpeedTreeUpdateSystem";
static const char* SCENE_WIND_SYSTEM = "WindSystem";
static const char* SCENE_WAVE_SYSTEM = "WaveSystem";
static const char* SCENE_SKELETON_SYSTEM = "SkeletonSystem";

//Render
static const char* RENDER_PASS_PREPARE_ARRAYS = "RenderPass::PrepareArrays";
static const char* RENDER_PASS_DRAW_LAYERS = "RenderPass::DrawLayers";
static const char* RENDER_PREPARE_LANDSCAPE = "Landscape::Prepare";

//RHI
static const char* RHI_RENDER_LOOP = "rhi::RenderLoop";
static const char* RHI_PRESENT = "rhi::Present";
static const char* RHI_DEVICE_PRESENT = "rhi::DevicePresent";
static const char* RHI_EXECUTE_QUEUED_CMDS = "rhi::ExecuteQueuedCmds";
static const char* RHI_EXECUTE_IMMEDIATE_CMDS = "rhi::ExecuteImmidiateCmds";
static const char* RHI_WAIT_IMMEDIATE_CMDS = "rhi::WaitImmediateCmd";
static const char* RHI_WAIT_FRAME_EXECUTION = "rhi::WaitFrameExecution";
static const char* RHI_CMD_BUFFER_EXECUTE = "rhi::cb::Execute";
static const char* RHI_WAIT_FRAME = "rhi::WaitFrame";
static const char* RHI_PROCESS_SCHEDULED_DELETE = "rhi::ProcessScheduledDelete";
};

namespace GPUMarkerName
{
static const char* GPU_FRAME = "GPUFrame";
static const char* RENDER_PASS_2D = "RenderPass2D";
static const char* RENDER_PASS_MAIN_3D = "RenderPassMain3D";
static const char* RENDER_PASS_WATER_REFLECTION = "RenderPassWaterRefl";
static const char* RENDER_PASS_WATER_REFRACTION = "RenderPassWaterRefr";
static const char* LANDSCAPE = "Landscape";
};
}; //ns DAVA