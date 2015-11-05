/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Settings/Settings.h"

using namespace DAVA;

const char Settings::Delimiter = '/';

const FastName Settings::General_DesinerName = FastName("General/DesignerName");
const FastName Settings::General_RecentFilesCount = FastName("General/RecentFilesCount");
const FastName Settings::General_RecentProjectsCount = FastName("General/RecentProjectsCount");
const FastName Settings::General_PreviewEnabled = FastName("General/PreviewEnabled");
const FastName Settings::General_OpenByDBClick = FastName("General/OpenByDoubleClick");
const FastName Settings::General_CompressionQuality = FastName("General/CompressionQuality");
const FastName Settings::General_MaterialEditor_SwitchColor0 = FastName("General/MaterialEditor/SwitchColor0");
const FastName Settings::General_MaterialEditor_SwitchColor1 = FastName("General/MaterialEditor/SwitchColor1");
const FastName Settings::General_MaterialEditor_LodColor0 = FastName("General/MaterialEditor/LodColor0");
const FastName Settings::General_MaterialEditor_LodColor1 = FastName("General/MaterialEditor/LodColor1");
const FastName Settings::General_MaterialEditor_LodColor2 = FastName("General/MaterialEditor/LodColor2");
const FastName Settings::General_MaterialEditor_LodColor3 = FastName("General/MaterialEditor/LodColor3");
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
const FastName Settings::Scene_RefreshLodForNonSolid = FastName("Scene/RefreshLodForNonSolid");
const FastName Settings::Scene_RememberForceParameters = FastName("Scene/RememberForceParameters");
const FastName Settings::Scene_SaveEmitters = FastName("Scene/SaveEmittersWithScene");
const FastName Settings::Scene_Sound_SoundObjectDraw = FastName("Scene/Sound/SoundObjectDraw");
const FastName Settings::Scene_Sound_SoundObjectBoxColor = FastName("Scene/Sound/SoundObjectBoxColor");
const FastName Settings::Scene_Sound_SoundObjectSphereColor = FastName("Scene/Sound/SoundObjectSphereColor");
const FastName Settings::General_Mouse_InvertWheel = FastName("General/Mouse/InvertWheel");
const FastName Settings::General_Mouse_WheelMoveCamera = FastName("General/Mouse/WheelMoveCamera");
const FastName Settings::InternalGroup = FastName("Internal");
const FastName Settings::Internal_TextureViewGPU = FastName("Internal/TextureViewGPU");
const FastName Settings::Internal_LastProjectPath = FastName("Internal/LastProjectPath");
const FastName Settings::Internal_EditorVersion = FastName("Internal/EditorVersion");
const FastName Settings::Internal_CubemapLastFaceDir = FastName("Internal/CubemapLastFaceDir");
const FastName Settings::Internal_CubemapLastProjDir = FastName("Internal/CubemapLastProjDir");
const FastName Settings::Internal_ParticleLastEmitterDir = FastName("Internal/ParticleLastEmitterDir");
const FastName Settings::Internal_RecentFiles = FastName("Internal/RecentFiles");
const FastName Settings::Internal_RecentProjects = FastName("Internal/RecentProjects");
const FastName Settings::Internal_MaterialsLightViewMode = FastName("Internal/MaterialsLightViewMode");
const FastName Settings::Internal_MaterialsShowLightmapCanvas = FastName("Internal/MaterialsShowLightmapCanvas");
const FastName Settings::Internal_LicenceAccepted = FastName("Internal/LicenceAccepted");
const FastName Settings::Internal_LODEditorMode = FastName("Internal/LODEditorMode");
const FastName Settings::Internal_ImageSplitterPath = FastName("Internal/ImageSplitterPath");
const FastName Settings::Internal_ImageSplitterPathSpecular = FastName("Internal/ImageSplitterPath_specular");
const FastName Settings::Internal_CustomPalette = FastName("Internal/CustomPalette");
const FastName Settings::Internal_LogWidget = FastName("Internal/LogWidget");
