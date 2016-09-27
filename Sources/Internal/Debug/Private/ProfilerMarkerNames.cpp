#pragma once

#include "Debug/ProfilerMarkerNames.h"

namespace DAVA
{
//CPU Markers
//Core flow
const char* CPUMarkerName::CORE_PROCESS_FRAME = "Core::SystemProcessFrame";
const char* CPUMarkerName::CORE_BEGIN_FRAME = "Core::BeginFrame";
const char* CPUMarkerName::CORE_END_FRAME = "Core::EndFrame";
const char* CPUMarkerName::CORE_JOB_MANAGER = "JobManager";
const char* CPUMarkerName::CORE_APP_UPDATE = "ApplicationCore::Update";
const char* CPUMarkerName::CORE_APP_DRAW = "ApplicationCore::Draw";
const char* CPUMarkerName::CORE_SOUND_SYSTEM = "SoundSystem";
const char* CPUMarkerName::CORE_ANIMATION_MANAGER = "AnimationManager";
const char* CPUMarkerName::CORE_UI_SYSTEM_UPDATE = "UI::Update";
const char* CPUMarkerName::CORE_UI_SYSTEM_DRAW = "UI::Draw";

//Scene
const char* CPUMarkerName::SCENE_UPDATE = "Scene::Update";
const char* CPUMarkerName::SCENE_DRAW = "Scene::Draw";
const char* CPUMarkerName::SCENE_STATIC_OCCLUSION_SYSTEM = "StaticOcclusionSystem";
const char* CPUMarkerName::SCENE_ANIMATION_SYSTEM = "AnimationSystem";
const char* CPUMarkerName::SCENE_UPDATE_SYSTEM_PRE_TRANSFORM = "UpdateSystem::PreTransform";
const char* CPUMarkerName::SCENE_UPDATE_SYSTEM_POST_TRANSFORM = "UpdateSystem::PostTransform";
const char* CPUMarkerName::SCENE_TRANSFORM_SYSTEM = "TransformSystem";
const char* CPUMarkerName::SCENE_LOD_SYSTEM = "LodSystem";
const char* CPUMarkerName::SCENE_SWITCH_SYSTEM = "SwitchSystem";
const char* CPUMarkerName::SCENE_PARTICLE_SYSTEM = "ParticleEffectSystem";
const char* CPUMarkerName::SCENE_SOUND_UPDATE_SYSTEM = "SoundUpdateSystem";
const char* CPUMarkerName::SCENE_RENDER_UPDATE_SYSTEM = "RenderUpdateSystem";
const char* CPUMarkerName::SCENE_ACTION_UPDATE_SYSTEM = "ActionUpdateSystem";
const char* CPUMarkerName::SCENE_DEBUG_RENDER_SYSTEM = "DebugRenderSystem";
const char* CPUMarkerName::SCENE_LANDSCAPE_SYSTEM = "LandscapeSystem";
const char* CPUMarkerName::SCENE_FOLIAGE_SYSTEM = "FoliageSystem";
const char* CPUMarkerName::SCENE_SPEEDTREE_SYSTEM = "SpeedTreeUpdateSystem";
const char* CPUMarkerName::SCENE_WIND_SYSTEM = "WindSystem";
const char* CPUMarkerName::SCENE_WAVE_SYSTEM = "WaveSystem";
const char* CPUMarkerName::SCENE_SKELETON_SYSTEM = "SkeletonSystem";

//Render
const char* CPUMarkerName::RENDER_PASS_PREPARE_ARRAYS = "RenderPass::PrepareArrays";
const char* CPUMarkerName::RENDER_PASS_DRAW_LAYERS = "RenderPass::DrawLayers";
const char* CPUMarkerName::RENDER_PREPARE_LANDSCAPE = "Landscape::Prepare";

//RHI
const char* CPUMarkerName::RHI_RENDER_LOOP = "rhi::RenderLoop";
const char* CPUMarkerName::RHI_PRESENT = "rhi::Present";
const char* CPUMarkerName::RHI_DEVICE_PRESENT = "rhi::DevicePresent";
const char* CPUMarkerName::RHI_EXECUTE_QUEUED_CMDS = "rhi::ExecuteQueuedCmds";
const char* CPUMarkerName::RHI_EXECUTE_IMMEDIATE_CMDS = "rhi::ExecuteImmidiateCmds";
const char* CPUMarkerName::RHI_WAIT_IMMEDIATE_CMDS = "rhi::WaitImmediateCmd";
const char* CPUMarkerName::RHI_WAIT_FRAME_EXECUTION = "rhi::WaitFrameExecution";
const char* CPUMarkerName::RHI_CMD_BUFFER_EXECUTE = "rhi::cb::Execute";
const char* CPUMarkerName::RHI_WAIT_FRAME = "rhi::WaitFrame";
const char* CPUMarkerName::RHI_PROCESS_SCHEDULED_DELETE = "rhi::ProcessScheduledDelete";

//GPU Markers
const char* GPUMarkerName::GPU_FRAME = "GPUFrame";
const char* GPUMarkerName::RENDER_PASS_2D = "RenderPass2D";
const char* GPUMarkerName::RENDER_PASS_MAIN_3D = "RenderPassMain3D";
const char* GPUMarkerName::RENDER_PASS_WATER_REFLECTION = "RenderPassWaterRefl";
const char* GPUMarkerName::RENDER_PASS_WATER_REFRACTION = "RenderPassWaterRefr";
const char* GPUMarkerName::LANDSCAPE = "Landscape";

}; //ns DAVA