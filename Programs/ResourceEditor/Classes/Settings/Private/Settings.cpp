#include "Classes/Settings/Settings.h"
#include "Classes/Settings/SettingsManager.h"

#include <Render/GPUFamilyDescriptor.h>

using namespace DAVA;

const char Settings::Delimiter = '/';

const FastName Settings::General_RecentFilesCount = FastName("General/RecentFilesCount");
const FastName Settings::General_RecentProjectsCount = FastName("General/RecentProjectsCount");
const FastName Settings::General_ReloadParticlesOnPojectOpening = FastName("General/ReloadParticlesOnProjectOpening");
const FastName Settings::General_PreviewEnabled = FastName("General/PreviewEnabled");
const FastName Settings::General_OpenByDBClick = FastName("General/OpenByDoubleClick");
const FastName Settings::General_CompressionQuality = FastName("General/CompressionQuality");
const FastName Settings::General_ShowErrorDialog = FastName("General/ShowDialogOnError");
const FastName Settings::General_MaterialEditor_SwitchColor0 = FastName("General/MaterialEditor/SwitchColor0");
const FastName Settings::General_MaterialEditor_SwitchColor1 = FastName("General/MaterialEditor/SwitchColor1");
const FastName Settings::General_MaterialEditor_LodColor0 = FastName("General/MaterialEditor/LodColor0");
const FastName Settings::General_MaterialEditor_LodColor1 = FastName("General/MaterialEditor/LodColor1");
const FastName Settings::General_MaterialEditor_LodColor2 = FastName("General/MaterialEditor/LodColor2");
const FastName Settings::General_MaterialEditor_LodColor3 = FastName("General/MaterialEditor/LodColor3");

const FastName Settings::General_ParticleEditor_ParticleDebugAlphaTheshold = FastName("General/ParticleEditor/ParticleDebugAlphaTheshold");

const FastName Settings::General_LODEditor_LodColor0 = FastName("General/LODEditor/LodColor0");
const FastName Settings::General_LODEditor_LodColor1 = FastName("General/LODEditor/LodColor1");
const FastName Settings::General_LODEditor_LodColor2 = FastName("General/LODEditor/LodColor2");
const FastName Settings::General_LODEditor_LodColor3 = FastName("General/LODEditor/LodColor3");
const FastName Settings::General_LODEditor_InactiveColor = FastName("General/LODEditor/InactiveColor");
const FastName Settings::General_LODEditor_FitSliders = FastName("General/LODEditor/FitSlidersToMaximumDistance");

const FastName Settings::General_HeighMaskTool_Color0 = FastName("General/HeighMaskTool/Color0");
const FastName Settings::General_HeighMaskTool_Color1 = FastName("General/HeighMaskTool/Color1");
const FastName Settings::General_ColorMultiplyMax = FastName("General/ColorPicker/Maximum multiplier");
const FastName Settings::General_AssetCache_UseCache = FastName("General/AssetCache/UseCache");
const FastName Settings::General_AssetCache_Ip = FastName("General/AssetCache/IP");
const FastName Settings::General_AssetCache_Port = FastName("General/AssetCache/Port");
const FastName Settings::General_AssetCache_Timeout = FastName("General/AssetCache/Timeout");
const FastName Settings::General_AutoConvertation = FastName("General/TextureBrowser/AutoConvertation");
const FastName Settings::Scene_GridStep = FastName("Scene/GridStep");
const FastName Settings::Scene_GridSize = FastName("Scene/GridSize");
const FastName Settings::Scene_CameraSpeed0 = FastName("Scene/CameraSpeed0");
const FastName Settings::Scene_CameraSpeed1 = FastName("Scene/CameraSpeed1");
const FastName Settings::Scene_CameraSpeed2 = FastName("Scene/CameraSpeed2");
const FastName Settings::Scene_CameraSpeed3 = FastName("Scene/CameraSpeed3");
const FastName Settings::Scene_CameraFOV = FastName("Scene/CameraFOV");
const FastName Settings::Scene_CameraNear = FastName("Scene/CameraNear");
const FastName Settings::Scene_CameraFar = FastName("Scene/CameraFar");
const FastName Settings::Scene_CameraHeightOnLandscape = FastName("Scene/HeightOnLandscape");
const FastName Settings::Scene_CameraHeightOnLandscapeStep = FastName("Scene/HeightOnLandscapeStep");
const FastName Settings::Scene_SelectionSequent = FastName("Scene/SelectionSequent");
const FastName Settings::Scene_SelectionOnClick = FastName("Scene/SelectionOnClick");
const FastName Settings::Scene_SelectionDrawMode = FastName("Scene/SelectionDrawMode");
const FastName Settings::Scene_CollisionDrawMode = FastName("Scene/CollisionDrawMode");
const FastName Settings::Scene_ModificationByGizmoOnly = FastName("Scene/ModificationByGizmoOnly");
const FastName Settings::Scene_GizmoScale = FastName("Scene/GizmoScale");
const FastName Settings::Scene_DebugBoxScale = FastName("Scene/DebugBoxScale");
const FastName Settings::Scene_DebugBoxUserScale = FastName("Scene/DebugBoxUserScale");
const FastName Settings::Scene_DebugBoxParticleScale = FastName("Scene/DebugBoxParticleScale");
const FastName Settings::Scene_DebugBoxWaypointScale = FastName("Scene/DebugBoxWaypointScale");
const FastName Settings::Scene_DragAndDropWithShift = FastName("Scene/Drag&DropInTreeWithShift");
const FastName Settings::Scene_AutoselectNewEntities = FastName("Scene/AutoselectNewEnities");
const FastName Settings::Scene_RememberForceParameters = FastName("Scene/RememberForceParameters");
const FastName Settings::Scene_SaveEmitters = FastName("Scene/SaveEmittersWithScene");
const FastName Settings::Scene_SaveStaticOcclusion = FastName("Scene/SaveAfterStaticOcclusion");
const FastName Settings::Scene_DefaultCustomColorIndex = FastName("Scene/DefaultCustomColorIndex");
const FastName Settings::Scene_Sound_SoundObjectDraw = FastName("Scene/Sound/SoundObjectDraw");
const FastName Settings::Scene_Sound_SoundObjectBoxColor = FastName("Scene/Sound/SoundObjectBoxColor");
const FastName Settings::Scene_Sound_SoundObjectSphereColor = FastName("Scene/Sound/SoundObjectSphereColor");
const FastName Settings::Scene_Grab_Size_Width = FastName("Scene/Grab Scene/Width");
const FastName Settings::Scene_Grab_Size_Height = FastName("Scene/Grab Scene/Height");
const FastName Settings::General_Mouse_InvertWheel = FastName("General/Mouse/InvertWheel");
const FastName Settings::General_Mouse_WheelMoveCamera = FastName("General/Mouse/WheelMoveCamera");
const FastName Settings::General_Mouse_WheelMoveIntensity = FastName("General/Mouse/WheelMoveIntensity");

const FastName Settings::InternalGroup = FastName("Internal");
const FastName Settings::Internal_TextureViewGPU = FastName("Internal/TextureViewGPU");
const FastName Settings::Internal_SpriteViewGPU = FastName("Internal/SpriteViewGPU");
const FastName Settings::Internal_LastProjectPath = FastName("Internal/LastProjectPath");
const FastName Settings::Internal_EditorVersion = FastName("Internal/EditorVersion");
const FastName Settings::Internal_CubemapLastFaceDir = FastName("Internal/CubemapLastFaceDir");
const FastName Settings::Internal_CubemapLastProjDir = FastName("Internal/CubemapLastProjDir");
const FastName Settings::Internal_ParticleLastSaveEmitterDir = FastName("Internal/ParticleLastEmitterDir");
const FastName Settings::Internal_ParticleLastLoadEmitterDir = FastName("Internal/ParticleLastLoadEmitterDir");
const FastName Settings::Internal_RecentFiles = FastName("Internal/RecentFiles");
const FastName Settings::Internal_RecentProjects = FastName("Internal/RecentProjects");
const FastName Settings::Internal_MaterialsLightViewMode = FastName("Internal/MaterialsLightViewMode");
const FastName Settings::Internal_MaterialsShowLightmapCanvas = FastName("Internal/MaterialsShowLightmapCanvas");
const FastName Settings::Internal_LicenceAccepted = FastName("Internal/LicenceAccepted");
const FastName Settings::Internal_LODEditor_Mode = FastName("Internal/LODEditorMode");
const FastName Settings::Internal_LODEditor_Recursive = FastName("Internal/LodEditor/Recursive");
const FastName Settings::Internal_ImageSplitterPath = FastName("Internal/ImageSplitterPath");
const FastName Settings::Internal_ImageSplitterPathSpecular = FastName("Internal/ImageSplitterPath_specular");
const FastName Settings::Internal_CustomPalette = FastName("Internal/CustomPalette");
const FastName Settings::Internal_LogWidget = FastName("Internal/LogWidget");
const FastName Settings::Internal_EnableSounds = FastName("Internal/EnableSounds");
const FastName Settings::Internal_Validate_Matrices = FastName("Internal/ValidateMatrices");
const FastName Settings::Internal_Validate_SameNames = FastName("Internal/ValidateSameNames");
const FastName Settings::Internal_Validate_CollisionProperties = FastName("Internal/ValidateCollisionProperties");
const FastName Settings::Internal_Validate_TexturesRelevance = FastName("Internal/ValidateTexturesRelevance");
const FastName Settings::Internal_Validate_MaterialGroups = FastName("Internal/ValidateMaterialGroups");
const FastName Settings::Internal_Validate_ShowConsole = FastName("Internal/ValidateShowConsole");
const FastName Settings::Internal_GizmoEnabled = FastName("Internal/GizmoEnabled");

DAVA::eGPUFamily Settings::GetGPUFormat()
{
    return static_cast<DAVA::eGPUFamily>(DAVA::GPUFamilyDescriptor::ConvertValueToGPU(SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsUInt32()));
}
