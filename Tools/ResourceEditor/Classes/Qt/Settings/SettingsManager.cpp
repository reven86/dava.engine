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


#include "SettingsManager.h"
#include "Scene/System/EditorMaterialSystem.h"
#include "TextureCompression/TextureConverter.h"

#include "Scene/System/SelectionSystem.h"
#include "Scene/System/CollisionSystem.h"

#include <QColor>
#include <QDebug>

// framework
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/VariantType.h"
#include "Render/RenderBase.h"

#include "AssetCache/AssetCache.h"

#define SETTINGS_CONFIG_FILE "~doc:/ResourceEditorOptions.archive"

SettingsManager::SettingsManager()
{
	Init();
	Load();
}

SettingsManager::~SettingsManager()
{
	Save();
}

void SettingsManager::Init()
{
	CreateValue(Settings::General_DesinerName, DAVA::VariantType(DAVA::String("nobody")));
    CreateValue(Settings::General_RecentFilesCount, DAVA::VariantType(static_cast<DAVA::int32>(5)));
    CreateValue(Settings::General_RecentProjectsCount, DAVA::VariantType(static_cast<DAVA::int32>(5)));
	CreateValue(Settings::General_PreviewEnabled, DAVA::VariantType(false));
    CreateValue(Settings::General_OpenByDBClick, DAVA::VariantType(true));
    CreateValue(Settings::General_CompressionQuality, DAVA::VariantType(static_cast<DAVA::int32>(DAVA::TextureConverter::ECQ_DEFAULT))
        , DAVA::InspDesc("Compression quality", GlobalEnumMap<DAVA::TextureConverter::eConvertQuality>::Instance()));

    CreateValue(Settings::General_MaterialEditor_SwitchColor0, DAVA::VariantType(DAVA::Color(0.0f, 1.0f, 0.0f, 1.0f)));
    CreateValue(Settings::General_MaterialEditor_SwitchColor1, DAVA::VariantType(DAVA::Color(1.0f, 0.0f, 0.0f, 1.0f)));
    CreateValue(Settings::General_MaterialEditor_LodColor0, DAVA::VariantType(DAVA::Color(0.9f, 0.9f, 0.9f, 1.0f)));
    CreateValue(Settings::General_MaterialEditor_LodColor1, DAVA::VariantType(DAVA::Color(0.7f, 0.7f, 0.7f, 1.0f)));
    CreateValue(Settings::General_MaterialEditor_LodColor2, DAVA::VariantType(DAVA::Color(0.5f, 0.5f, 0.5f, 1.0f)));
    CreateValue(Settings::General_MaterialEditor_LodColor3, DAVA::VariantType(DAVA::Color(0.3f, 0.3f, 0.3f, 1.0f)));
    CreateValue(Settings::General_HeighMaskTool_Color0, DAVA::VariantType(DAVA::Color(0.5f, 0.5f, 0.5f, 1.0f)));
    CreateValue(Settings::General_HeighMaskTool_Color1, DAVA::VariantType(DAVA::Color(0.0f, 0.0f, 0.0f, 1.0f)));

    CreateValue(Settings::General_AssetCache_UseCache, DAVA::VariantType(false));
    CreateValue(Settings::General_AssetCache_Ip, DAVA::VariantType(DAVA::String("")));
    CreateValue(Settings::General_AssetCache_Port, DAVA::VariantType(DAVA::Format("%d", DAVA::AssetCache::ASSET_SERVER_PORT)));
    CreateValue(Settings::General_AssetCache_Timeout, DAVA::VariantType(DAVA::String("")));

	CreateValue(Settings::Scene_GridStep, DAVA::VariantType(10.0f));
	CreateValue(Settings::Scene_GridSize, DAVA::VariantType(600.0f));
	CreateValue(Settings::Scene_CameraSpeed0, DAVA::VariantType(35.0f));
	CreateValue(Settings::Scene_CameraSpeed1, DAVA::VariantType(100.0f));
	CreateValue(Settings::Scene_CameraSpeed2, DAVA::VariantType(250.0f));
	CreateValue(Settings::Scene_CameraSpeed3, DAVA::VariantType(400.0f));
	CreateValue(Settings::Scene_CameraFOV, DAVA::VariantType(70.0f));
	CreateValue(Settings::Scene_CameraNear, DAVA::VariantType(1.0f));
	CreateValue(Settings::Scene_CameraFar, DAVA::VariantType(5000.0f));
    CreateValue(Settings::Scene_CameraHeightOnLandscape, DAVA::VariantType(2.0f));
    CreateValue(Settings::Scene_CameraHeightOnLandscapeStep, DAVA::VariantType(0.5f));
    CreateValue(Settings::Scene_SelectionSequent, DAVA::VariantType(false));
    CreateValue(Settings::Scene_SelectionDrawMode, DAVA::VariantType(static_cast<DAVA::int32>(SS_DRAW_DEFAULT)), DAVA::InspDesc("Selection draw modes", GlobalEnumMap<SelectionSystemDrawMode>::Instance(), DAVA::InspDesc::T_FLAGS));
    CreateValue(Settings::Scene_CollisionDrawMode, DAVA::VariantType(static_cast<DAVA::int32>(CS_DRAW_DEFAULT)), DAVA::InspDesc("Collision draw modes", GlobalEnumMap<CollisionSystemDrawMode>::Instance(), DAVA::InspDesc::T_FLAGS));
    CreateValue(Settings::Scene_ModificationByGizmoOnly, DAVA::VariantType(false));
    CreateValue(Settings::Scene_GizmoScale, DAVA::VariantType(DAVA::float32(1.0)));
    CreateValue(Settings::Scene_DebugBoxScale, DAVA::VariantType(DAVA::float32(1.0)));
    CreateValue(Settings::Scene_DebugBoxUserScale, DAVA::VariantType(DAVA::float32(1.0)));
    CreateValue(Settings::Scene_DebugBoxParticleScale, DAVA::VariantType(DAVA::float32(1.0)));
    CreateValue(Settings::Scene_DebugBoxWaypointScale, DAVA::VariantType(DAVA::float32(1.0)));
    CreateValue(Settings::Scene_DragAndDropWithShift, DAVA::VariantType(false));
    CreateValue(Settings::Scene_AutoselectNewEntities, DAVA::VariantType(true));
    CreateValue(Settings::Scene_RefreshLodForNonSolid, DAVA::VariantType(true));
    CreateValue(Settings::Scene_RememberForceParameters, DAVA::VariantType(false));
    CreateValue(Settings::Scene_SaveEmitters, DAVA::VariantType(false));

    CreateValue(Settings::Scene_Sound_SoundObjectDraw, DAVA::VariantType(false));
    CreateValue(Settings::Scene_Sound_SoundObjectBoxColor, DAVA::VariantType(DAVA::Color(0.0f, 0.8f, 0.4f, 0.2f)));
    CreateValue(Settings::Scene_Sound_SoundObjectSphereColor, DAVA::VariantType(DAVA::Color(0.0f, 0.8f, 0.4f, 0.1f)));

    CreateValue( Settings::General_Mouse_WheelMoveCamera, DAVA::VariantType( true ) );
    CreateValue( Settings::General_Mouse_InvertWheel, DAVA::VariantType( false ) );

    CreateValue(Settings::Internal_TextureViewGPU, DAVA::VariantType(static_cast<DAVA::uint32>(DAVA::GPU_ORIGIN)));
    CreateValue(Settings::Internal_LastProjectPath, DAVA::VariantType(DAVA::FilePath()));
	CreateValue(Settings::Internal_EditorVersion, DAVA::VariantType(DAVA::String("local build")));
	CreateValue(Settings::Internal_CubemapLastFaceDir, DAVA::VariantType(DAVA::FilePath()));
	CreateValue(Settings::Internal_CubemapLastProjDir, DAVA::VariantType(DAVA::FilePath()));
    CreateValue(Settings::Internal_ParticleLastEmitterDir, DAVA::VariantType(DAVA::FilePath()));

	CreateValue(Settings::Internal_RecentFiles, DAVA::VariantType(static_cast<DAVA::KeyedArchive *>(nullptr)));
    CreateValue(Settings::Internal_RecentProjects, DAVA::VariantType(static_cast<DAVA::KeyedArchive *>(nullptr)));
    
    CreateValue(Settings::Internal_MaterialsLightViewMode, DAVA::VariantType(static_cast<DAVA::int32>(EditorMaterialSystem::LIGHTVIEW_ALL)));
    CreateValue(Settings::Internal_MaterialsShowLightmapCanvas, DAVA::VariantType(static_cast<bool>(false)));
    CreateValue(Settings::Internal_LicenceAccepted, DAVA::VariantType(static_cast<bool>(false)));
	CreateValue(Settings::Internal_LODEditorMode, DAVA::VariantType(static_cast<bool>(false)));
    CreateValue(DAVA::FastName("Internal/RunActionEventWidget/CurrentType"), DAVA::VariantType(static_cast<DAVA::uint32>(0)));
    CreateValue(DAVA::FastName("Internal/Beast/LightmapsDefaultDir"), DAVA::VariantType(DAVA::String("lightmaps")));
    CreateValue(Settings::Internal_ImageSplitterPath, DAVA::VariantType(DAVA::String("")));
    CreateValue(Settings::Internal_ImageSplitterPathSpecular, DAVA::VariantType(DAVA::String("")));

    const DAVA::int32 nColors = Qt::darkYellow - Qt::black + 1;
    DAVA::uint32 colors[nColors];   // Init from Qt::GlobalColor
    for (int i = 0; i < nColors; i++)
    {
        colors[i] = QColor(Qt::GlobalColor(i + Qt::black)).rgba();
    }
    CreateValue(Settings::Internal_CustomPalette, DAVA::VariantType( reinterpret_cast<DAVA::uint8 *>(colors), nColors * sizeof(*colors) ));
    CreateValue(Settings::General_ColorMultiplyMax, DAVA::VariantType(static_cast<DAVA::float32>(2.0)));
    CreateValue(Settings::Internal_LogWidget, DAVA::VariantType(nullptr, 0));
}

DAVA::VariantType SettingsManager::GetValue(const DAVA::FastName& path)
{
    DAVA::FastNameMap<SettingsNode>::iterator i = SettingsManager::Instance()->settingsMap.find(path);
    DVASSERT(i != SettingsManager::Instance()->settingsMap.end() && "No such setting path");

    return i->second.value;
}

void SettingsManager::SetValue(const DAVA::FastName& path, const DAVA::VariantType &value)
{
    DAVA::FastNameMap<SettingsNode>::iterator i = SettingsManager::Instance()->settingsMap.find(path);
    DVASSERT(i != SettingsManager::Instance()->settingsMap.end() && "No such setting path");
    DVASSERT(i->second.value.type == value.type && "Setting different type");

    i->second.value.SetVariant(value);

    SettingsManager::Instance()->Save();
}

size_t SettingsManager::GetSettingsCount()
{
    return SettingsManager::Instance()->settingsMap.size();
}

SettingsNode* SettingsManager::GetSettingsNode(const DAVA::FastName &name)
{
    DVASSERT(0 != SettingsManager::Instance()->settingsMap.count(name));
    return &SettingsManager::Instance()->settingsMap.at(name);
}

DAVA::FastName SettingsManager::GetSettingsName(size_t index)
{
    DVASSERT(index < SettingsManager::Instance()->settingsOrder.size());
    return SettingsManager::Instance()->settingsOrder[index];
}

bool SettingsManager::CustomTextureViewGPULoad(const DAVA::String & paramName, const DAVA::VariantType & src_value, DAVA::VariantType & dstValue)
{
    if (DAVA::VariantType::TYPE_INT32 == src_value.GetType() && paramName == Settings::Internal_TextureViewGPU.c_str())
    {
        DAVA::eGPUFamily gpuFamilyRead = DAVA::GPUFamilyDescriptor::ConvertValueToGPU(src_value.AsInt32());
        DAVA::uint32 valueToVariant = static_cast<DAVA::uint32>(gpuFamilyRead);
        dstValue.SetVariant(DAVA::VariantType(valueToVariant));
        return true;
    }
    return false;
}

void SettingsManager::Load()
{
	DAVA::KeyedArchive* toLoad = new DAVA::KeyedArchive();
	if(toLoad->Load(SETTINGS_CONFIG_FILE))
	{
        DAVA::FastNameMap<SettingsNode>::iterator i;
        DAVA::FastNameMap<SettingsNode>::iterator end = settingsMap.end();
        for(i = settingsMap.begin(); i != end; ++i)
        {
            SettingsNode *node = &i->second;
            DAVA::String name = i->first.c_str();

            if(toLoad->IsKeyExists(name))
            {
                DAVA::VariantType* sourceValue = toLoad->GetVariant(name);

                // try to set texture view gpu custom way.
                if (CustomTextureViewGPULoad(name, *sourceValue, node->value))
                {
                    continue;
                }
                // Not setted. Use general setter.
                if (sourceValue->type == node->value.type)
                {
                    node->value.SetVariant(*sourceValue);
                }
            }
        }
	}

    SafeRelease(toLoad);
}

void SettingsManager::Save()
{
	DAVA::KeyedArchive* toSave = new DAVA::KeyedArchive();

    DAVA::FastNameMap<SettingsNode>::iterator i;
    DAVA::FastNameMap<SettingsNode>::iterator end = settingsMap.end();
    for(i = settingsMap.begin(); i != end; ++i)
    {
        SettingsNode *node = &i->second;
        DAVA::String name = i->first.c_str();

        toSave->SetVariant(name, node->value);
    }

	toSave->Save(SETTINGS_CONFIG_FILE);
    toSave->Release();
}

void SettingsManager::CreateValue(const DAVA::FastName& pathName, const DAVA::VariantType &defaultValue, const DAVA::InspDesc &description)
{
    DVASSERT(pathName.IsValid());
    
    settingsMap[pathName].value = defaultValue;
    settingsMap[pathName].desc = description;

    settingsOrder.push_back(pathName);
}

void SettingsManager::ResetPerProjectSettings()
{
    SetValue(Settings::Internal_ParticleLastEmitterDir, DAVA::VariantType(DAVA::FilePath()));
}

void SettingsManager::ResetToDefault()
{
    SettingsManager::Instance()->Init();
}

void SettingsManager::UpdateGPUSettings()
{
    DAVA::uint32 oldGpu = GetValue(Settings::Internal_TextureViewGPU).AsUInt32();
    DAVA::uint32 newValue = DAVA::GPUFamilyDescriptor::ConvertValueToGPU(oldGpu);
    DAVA::VariantType newGpu = DAVA::VariantType(newValue);
    SetValue(Settings::Internal_TextureViewGPU, newGpu);
}

