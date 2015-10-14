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


#ifndef __RESOURCEEDITORQT__SETTINGS_MANAGER__
#define __RESOURCEEDITORQT__SETTINGS_MANAGER__

#include <QObject>
#include "DAVAEngine.h"
#include "FileSystem/VariantType.h"

namespace Settings
{
    static const char Delimiter('/');

    static const DAVA::FastName General_DesinerName("General/DesignerName");
    static const DAVA::FastName General_RecentFilesCount("General/RecentFilesCount");
    static const DAVA::FastName General_RecentProjectsCount("General/RecentProjectsCount");
	static const DAVA::FastName General_PreviewEnabled("General/PreviewEnabled");
    static const DAVA::FastName General_OpenByDBClick("General/OpenByDoubleClick");
    static const DAVA::FastName General_CompressionQuality("General/CompressionQuality");

    static const DAVA::FastName General_MaterialEditor_SwitchColor0("General/MaterialEditor/SwitchColor0");
    static const DAVA::FastName General_MaterialEditor_SwitchColor1("General/MaterialEditor/SwitchColor1");
    static const DAVA::FastName General_MaterialEditor_LodColor0("General/MaterialEditor/LodColor0");
    static const DAVA::FastName General_MaterialEditor_LodColor1("General/MaterialEditor/LodColor1");
    static const DAVA::FastName General_MaterialEditor_LodColor2("General/MaterialEditor/LodColor2");
    static const DAVA::FastName General_MaterialEditor_LodColor3("General/MaterialEditor/LodColor3");
    
    static const DAVA::FastName General_HeighMaskTool_Color0("General/HeighMaskTool/Color0");
    static const DAVA::FastName General_HeighMaskTool_Color1("General/HeighMaskTool/Color1");

    static const DAVA::FastName General_ColorMultiplyMax("General/ColorPicker/Maximum multiplier");

    static const DAVA::FastName General_AssetCache_UseCache("General/AssetCache/UseCache");
    static const DAVA::FastName General_AssetCache_Ip("General/AssetCache/IP");
    static const DAVA::FastName General_AssetCache_Port("General/AssetCache/Port");
    static const DAVA::FastName General_AssetCache_Timeout("General/AssetCache/Timeout");

    static const DAVA::FastName Scene_GridStep("Scene/GridStep");
    static const DAVA::FastName Scene_GridSize("Scene/GridSize");
    static const DAVA::FastName Scene_CameraSpeed0("Scene/CameraSpeed0");
    static const DAVA::FastName Scene_CameraSpeed1("Scene/CameraSpeed1");
    static const DAVA::FastName Scene_CameraSpeed2("Scene/CameraSpeed2");
    static const DAVA::FastName Scene_CameraSpeed3("Scene/CameraSpeed3");
    static const DAVA::FastName Scene_CameraFOV("Scene/CameraFOV");
    static const DAVA::FastName Scene_CameraNear("Scene/CameraNear");
	static const DAVA::FastName Scene_CameraFar("Scene/CameraFar");
    static const DAVA::FastName Scene_CameraHeightOnLandscape("Scene/HeightOnLandscape");
    static const DAVA::FastName Scene_CameraHeightOnLandscapeStep("Scene/HeightOnLandscapeStep");
    static const DAVA::FastName Scene_SelectionSequent("Scene/SelectionSequent");
    static const DAVA::FastName Scene_SelectionDrawMode("Scene/SelectionDrawMode");
    static const DAVA::FastName Scene_CollisionDrawMode("Scene/CollisionDrawMode");
    static const DAVA::FastName Scene_ModificationByGizmoOnly("Scene/ModificationByGizmoOnly");
    static const DAVA::FastName Scene_GizmoScale("Scene/GizmoScale");
    static const DAVA::FastName Scene_DebugBoxScale("Scene/DebugBoxScale");
    static const DAVA::FastName Scene_DebugBoxUserScale("Scene/DebugBoxUserScale");
    static const DAVA::FastName Scene_DebugBoxParticleScale("Scene/DebugBoxParticleScale");
    static const DAVA::FastName Scene_DebugBoxWaypointScale("Scene/DebugBoxWaypointScale");
    static const DAVA::FastName Scene_DragAndDropWithShift("Scene/Drag&DropInTreeWithShift");
    static const DAVA::FastName Scene_AutoselectNewEntities("Scene/AutoselectNewEnities");
    static const DAVA::FastName Scene_RefreshLodForNonSolid("Scene/RefreshLodForNonSolid");
    static const DAVA::FastName Scene_RememberForceParameters("Scene/RememberForceParameters");
    static const DAVA::FastName Scene_SaveEmitters("Scene/SaveEmittersWithScene");

    static const DAVA::FastName Scene_Sound_SoundObjectDraw("Scene/Sound/SoundObjectDraw");
    static const DAVA::FastName Scene_Sound_SoundObjectBoxColor("Scene/Sound/SoundObjectBoxColor");
    static const DAVA::FastName Scene_Sound_SoundObjectSphereColor("Scene/Sound/SoundObjectSphereColor");

    static const DAVA::FastName General_Mouse_InvertWheel( "General/Mouse/InvertWheel" );
    static const DAVA::FastName General_Mouse_WheelMoveCamera( "General/Mouse/WheelMoveCamera" );

    // this settings won't be shown in settings dialog
    // and are used only by application
    static const DAVA::FastName InternalGroup("Internal");
    static const DAVA::FastName Internal_TextureViewGPU("Internal/TextureViewGPU");
	static const DAVA::FastName Internal_LastProjectPath("Internal/LastProjectPath");    
	static const DAVA::FastName Internal_EditorVersion("Internal/EditorVersion");
	static const DAVA::FastName Internal_CubemapLastFaceDir("Internal/CubemapLastFaceDir");
	static const DAVA::FastName Internal_CubemapLastProjDir("Internal/CubemapLastProjDir");
    static const DAVA::FastName Internal_ParticleLastEmitterDir("Internal/ParticleLastEmitterDir");
	static const DAVA::FastName Internal_RecentFiles("Internal/RecentFiles");
    static const DAVA::FastName Internal_RecentProjects("Internal/RecentProjects");
    static const DAVA::FastName Internal_MaterialsLightViewMode("Internal/MaterialsLightViewMode");
    static const DAVA::FastName Internal_MaterialsShowLightmapCanvas("Internal/MaterialsShowLightmapCanvas");
    static const DAVA::FastName Internal_LicenceAccepted("Internal/LicenceAccepted");
	static const DAVA::FastName Internal_LODEditorMode("Internal/LODEditorMode");
    static const DAVA::FastName Internal_ImageSplitterPath("Internal/ImageSplitterPath");
    static const DAVA::FastName Internal_ImageSplitterPathSpecular("Internal/ImageSplitterPath_specular");

    static const DAVA::FastName Internal_CustomPalette("Internal/CustomPalette");
    static const DAVA::FastName Internal_LogWidget("Internal/LogWidget");
};

struct SettingsNode
{
    DAVA::VariantType value;
    DAVA::InspDesc desc;
};

class SettingsManager: public DAVA::Singleton<SettingsManager>
{
public:
	SettingsManager();
	~SettingsManager();

    static DAVA::VariantType GetValue(const DAVA::FastName& path);
	static void SetValue(const DAVA::FastName& path, const DAVA::VariantType &value);

    static DAVA::VariantType GetValue(const DAVA::String& path) { return GetValue(DAVA::FastName(path)); }
    static void SetValue(const DAVA::String& path, const DAVA::VariantType &value) { SetValue(DAVA::FastName(path), value); } 

    static size_t GetSettingsCount();
    static DAVA::FastName GetSettingsName(size_t index);
    static SettingsNode* GetSettingsNode(const DAVA::FastName &name);

    static void ResetPerProjectSettings();

    static void ResetToDefault();

    DAVA_DEPRECATED(static void UpdateGPUSettings());
    
protected:
    DAVA::Vector<DAVA::FastName> settingsOrder;
    DAVA::FastNameMap<SettingsNode> settingsMap;

    void Init();
    void Save();
	void Load();
    void CreateValue(const DAVA::FastName& path, const DAVA::VariantType &defaultValue, const DAVA::InspDesc &description = DAVA::InspDesc(""));
    DAVA_DEPRECATED(bool CustomTextureViewGPULoad(const DAVA::String& paramName, const DAVA::VariantType& src_value, DAVA::VariantType& dstValue));
};

#endif /* defined(__RESOURCEEDITORQT__SETTINGS_MANAGER__) */
