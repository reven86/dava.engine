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
#include "Deprecated/ControlsFactory.h"
#include "Render/RenderManager.h"
#include "Scene/System/EditorMaterialSystem.h"
#include <QHeaderView>

#define CONFIG_FILE						"~doc:/ResourceEditorOptions.archive"
#define SETTINGS_VERSION_NUMBER			1
#define SETTINGS_VERSION_KEY			"settingsVersion"

const SettingRow SETTINGS_GROUP_GENERAL_MAP[] =
{
	SettingRow("DesignerName", DAVA::VariantType(DAVA::String("nobody"))),
	SettingRow("PreviewDialogEnabled", DAVA::VariantType(false)),
    SettingRow("SequentSelection", DAVA::VariantType(false)),
    SettingRow("SwitchColor0", DAVA::VariantType(DAVA::Color(0.0f, 1.0f, 0.0f, 1.0f))),
    SettingRow("SwitchColor1", DAVA::VariantType(DAVA::Color(1.0f, 0.0f, 0.0f, 1.0f))),
    SettingRow("LodColor0", DAVA::VariantType(DAVA::Color(0.9f, 0.9f, 0.9f, 1.0f))),
    SettingRow("LodColor1", DAVA::VariantType(DAVA::Color(0.7f, 0.7f, 0.7f, 1.0f))),
    SettingRow("LodColor2", DAVA::VariantType(DAVA::Color(0.5f, 0.5f, 0.5f, 1.0f))),
    SettingRow("LodColor3", DAVA::VariantType(DAVA::Color(0.3f, 0.3f, 0.3f, 1.0f))),
};

const SettingRow SETTINGS_GROUP_DEFAULT_MAP[] =
{
	SettingRow("GridStep", DAVA::VariantType(10.0f)),
	SettingRow("CameraSpeedValue_0", DAVA::VariantType(35.0f)),
	SettingRow("CameraSpeedValue_1", DAVA::VariantType(100.0f)),
	SettingRow("CameraSpeedValue_2", DAVA::VariantType(250.0f)),
	SettingRow("CameraSpeedValue_3", DAVA::VariantType(400.0f)),
	SettingRow("DefaultCameraFOV", DAVA::VariantType(70.0f)),
    SettingRow("DefaultLandscapeSize", DAVA::VariantType(600.0f)),
    SettingRow("DefaultLandscapeHeight", DAVA::VariantType(50.0f)),
	SettingRow("DefaultFogColor", DAVA::VariantType(DAVA::Color(1.f, 0, 0.f, 1.f))),
	SettingRow("DefaultFogDensity", DAVA::VariantType(1.0f)),
};

const SettingRow SETTINGS_GROUP_INTERNAL_MAP[] =
{
	SettingRow("LastProjectPath", DAVA::VariantType(DAVA::FilePath())),
	SettingRow("TextureViewGPU", DAVA::VariantType(GPU_UNKNOWN)),
	SettingRow("editor.version", DAVA::VariantType(DAVA::String("local build"))),
	SettingRow("cubemap_last_face_dir", DAVA::VariantType(DAVA::FilePath())),
	SettingRow("cubemap_last_proj_dir", DAVA::VariantType(DAVA::FilePath())),
	SettingRow("recentFiles", DAVA::VariantType((KeyedArchive *)NULL)),
    SettingRow("recentFilesListCount", DAVA::VariantType(5)),
    SettingRow("materialsLightViewMode", DAVA::VariantType(EditorMaterialSystem::LIGHTVIEW_ALL)),
    SettingRow("materialsShowLightmapCanvas", DAVA::VariantType(false)),
    SettingRow( ResourceEditor::SETTINGS_LICENCE_ACCEPTED, DAVA::VariantType(false)),
};

SettingsManager::SettingsManager()
{
	settings = new KeyedArchive();

	InitSettingsGroup(GENERAL, SETTINGS_GROUP_GENERAL_MAP, sizeof(SETTINGS_GROUP_GENERAL_MAP) / sizeof(SettingRow));
	InitSettingsGroup(DEFAULT, SETTINGS_GROUP_DEFAULT_MAP, sizeof(SETTINGS_GROUP_DEFAULT_MAP) / sizeof(SettingRow));
	InitSettingsGroup(INTERNAL, SETTINGS_GROUP_INTERNAL_MAP, sizeof(SETTINGS_GROUP_INTERNAL_MAP) / sizeof(SettingRow));
	
	settings->SetUInt32(SETTINGS_VERSION_KEY, SETTINGS_VERSION_NUMBER);
	
	Load();
}

SettingsManager::~SettingsManager()
{
	Save();
	SafeRelease(settings);
}

void SettingsManager::InitSettingsGroup(SettingsManager::eSettingsGroups groupID, const SettingRow* groupMap, DAVA::uint32 mapSize)
{
	KeyedArchive* groupSettings = new KeyedArchive();
	
	for (uint32 i = 0; i < mapSize; ++i )
	{
		groupSettings->SetVariant( groupMap[i].key, DAVA::VariantType(groupMap[i].defValue) );
	}
	
	settings->SetArchive(GetNameOfGroup(groupID), groupSettings);
	SafeRelease(groupSettings);
}

void SettingsManager::Load()
{
	KeyedArchive* loadedArchive = new KeyedArchive();
	if(!loadedArchive->Load(CONFIG_FILE))
	{
        loadedArchive->Release();
		return;
	}
	bool isSettingsFileVersionActual = loadedArchive->IsKeyExists(SETTINGS_VERSION_KEY);
	// perform synchronization: update present in settings elements from file
	for (uint32 i = 0; i < GROUPS_COUNT; ++i)
	{
		KeyedArchive* presentSettingsGroup = GetSettingsGroup((eSettingsGroups)i);
		KeyedArchive* archiveFromFile = isSettingsFileVersionActual ?
		loadedArchive->GetArchive(GetNameOfGroup((eSettingsGroups)i)) : loadedArchive;
		
		if(NULL == archiveFromFile)
		{
			continue;
		}
		
		Map<String, VariantType*> predefinedSettingsGroupMap = presentSettingsGroup->GetArchieveData();
		DAVA::Map<DAVA::String, DAVA::VariantType*>::iterator it = predefinedSettingsGroupMap.begin();
		DAVA::Map<DAVA::String, DAVA::VariantType*>::iterator end = predefinedSettingsGroupMap.end();
		
		for(; it != end; ++it)
		{
			if(archiveFromFile->IsKeyExists(it->first))
			{
				presentSettingsGroup->SetVariant(it->first, *archiveFromFile->GetVariant(it->first));
			}
		}
	}
	
	SafeRelease(loadedArchive);
}

DAVA::VariantType SettingsManager::GetValue(const DAVA::String& _name, eSettingsGroups _group) const
{
	KeyedArchive* foundedGroupSettings = settings->GetArchive(GetNameOfGroup(_group));
	DVASSERT(foundedGroupSettings->IsKeyExists(_name));
	return *foundedGroupSettings->GetVariant(_name);
}

void SettingsManager::SetValue(const DAVA::String& _name, const DAVA::VariantType& _value, eSettingsGroups group)
{
	KeyedArchive* foundedGroupSettings = settings->GetArchive(GetNameOfGroup(group));
	DVASSERT(foundedGroupSettings->IsKeyExists(_name));
	foundedGroupSettings->SetVariant(_name, _value);
	Save();
}

void SettingsManager::Save()
{
	settings->Save(CONFIG_FILE);
}

DAVA::KeyedArchive* SettingsManager::GetSettingsGroup(eSettingsGroups group)
{
	String nameOfGroup = GetNameOfGroup(group);
	return settings->GetArchive(nameOfGroup);
}

DAVA::String SettingsManager::GetNameOfGroup(eSettingsGroups group) const
{
	static struct
	{
		eSettingsGroups groupKey;
		const char* groupName;
	}
	groupNamesMap[] =
	{
		{GENERAL, "General\0"},
		{DEFAULT, "Scene default\0"},
		{INTERNAL, "Internal\0"}
	};
	
	uint32 namesCount = COUNT_OF(groupNamesMap);
	DVASSERT(namesCount == GROUPS_COUNT);
	
	for (uint32 i = 0; i < namesCount; i ++)
	{
		if (groupNamesMap[i].groupKey == group)
		{
			return groupNamesMap[i].groupName;
		}
	}
	
	DVASSERT(false);
	return DAVA::String();
}
