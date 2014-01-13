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
#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/ControlsFactory.h"
#include "Render/RenderManager.h"
#include <QHeaderView>
#include "../StringConstants.h"

#define BEGIN_GETTERS_MAP 	DAVA::VariantType defValue;\
	DAVA::String validKey=_name;\
	if(0){}

#define END_GETTERS_MAP 	if(settings->IsKeyExists(validKey))\
	{\
		return *settings->GetVariant(validKey);\
	}\
	else\
	{\
		return defValue;\
	}\

#define SIMPLE_GETTER(settingsName, settingsValue) else if(_name == settingsName)\
{\
	defValue = DAVA::VariantType(settingsValue);\
}

#define EXTENDED_GETTER(settingsName, metodName) else if(_name == settingsName)\
{\
	metodName(additionalArguments, defValue, validKey);\
}

#define BEGIN_SETTERS_MAP DAVA::String validKey= _name;\
	if(0){}


#define EXTENDED_SETTER(settingsName, type, metodName) else if(_name == settingsName)\
{	DVASSERT(_value.GetType() == type );\
	metodName(additionalArguments, _value, validKey);}

#define SIMPLE_SETTER(settingsName, type) else if(_name == settingsName)\
{DVASSERT(_value.GetType() == type );}

#define END_SETTERS_MAP DAVA::VariantType *presentSetting = NULL;\
if(settings->IsKeyExists(validKey)){\
	presentSetting = settings->GetVariant(validKey);}\
if(presentSetting == NULL ||_value != *presentSetting){\
	settings->SetVariant(validKey,_value);\
	Save();\
	emit ConfigurationChanged(validKey);}

#define CONFIG_FILE "~doc:/ResourceEditorOptions.archive"

SettingsManager::SettingsManager()
{
	settings = new KeyedArchive();
	
	settings->Load(CONFIG_FILE);
		
	ApplyOptions();
}

SettingsManager::~SettingsManager()
{
	SafeRelease(settings);
}

void SettingsManager::SetValue(const DAVA::String& _name, const DAVA::VariantType& _value, DAVA::List<DAVA::VariantType>& additionalArguments)
{	
	BEGIN_SETTERS_MAP
		SIMPLE_SETTER(ResourceEditor::SETTINGS_3D_DATA_SOURCEPATH,	DAVA::VariantType::TYPE_STRING)
		SIMPLE_SETTER(ResourceEditor::SETTINGS_PROJECT_PATH,		DAVA::VariantType::TYPE_STRING)
		EXTENDED_SETTER(ResourceEditor::SETTINGS_CAMERA_SPEED_INDEX,DAVA::VariantType::TYPE_INT32,CameraSpeedIndexCheck)
		EXTENDED_SETTER(ResourceEditor::SETTINGS_CAMERA_SPEED_VALUE,DAVA::VariantType::TYPE_FLOAT,ResolveCameraSpeedValueKey)
		SIMPLE_SETTER(ResourceEditor::SETTINGS_SCREEN_WIDTH,		DAVA::VariantType::TYPE_INT32)
		SIMPLE_SETTER(ResourceEditor::SETTINGS_SCREEN_HEIGHT,		DAVA::VariantType::TYPE_INT32)
		SIMPLE_SETTER(ResourceEditor::SETTINGS_LANGUAGE,			DAVA::VariantType::TYPE_STRING)
		SIMPLE_SETTER(ResourceEditor::SETTINGS_SHOW_OUTPUT,			DAVA::VariantType::TYPE_BOOLEAN)
		SIMPLE_SETTER(ResourceEditor::SETTINGS_LEFT_PANEL_WIDTH,	DAVA::VariantType::TYPE_INT32)
		SIMPLE_SETTER(ResourceEditor::SETTINGS_RIGHT_PANEL_WIDTH,	DAVA::VariantType::TYPE_INT32)
		EXTENDED_SETTER(ResourceEditor::SETTINGS_LOD_LAYER,			DAVA::VariantType::TYPE_FLOAT, ResolveLodLevelKey)
		EXTENDED_SETTER(ResourceEditor::SETTINGS_LAST_OPENED_FILE,	DAVA::VariantType::TYPE_STRING, AddLastOpenedFile)
		SIMPLE_SETTER(ResourceEditor::SETTINGS_DRAW_GRID,			DAVA::VariantType::TYPE_BOOLEAN)
		EXTENDED_SETTER(ResourceEditor::SETTINGS_ENABLE_IMPOSTERS,	DAVA::VariantType::TYPE_BOOLEAN, SetImposterOption)
		SIMPLE_SETTER(ResourceEditor::SETTINGS_TEXTURE_VIEW_GPU,	DAVA::VariantType::TYPE_INT32)
		SIMPLE_SETTER(ResourceEditor::SETTINGS_MATERIALS_AMBIENT,	DAVA::VariantType::TYPE_COLOR)
		SIMPLE_SETTER(ResourceEditor::SETTINGS_MATERIALS_DIFFUSE,	DAVA::VariantType::TYPE_COLOR)
		SIMPLE_SETTER(ResourceEditor::SETTINGS_MATERIALS_SPECULAR,	DAVA::VariantType::TYPE_COLOR)
		SIMPLE_SETTER(ResourceEditor::SETTINGS_DESIGNER_NAME,		DAVA::VariantType::TYPE_STRING)
		SIMPLE_SETTER(ResourceEditor::SETTINGS_PREVIEW_DIALOG_ENABLED, DAVA::VariantType::TYPE_BOOLEAN)
	END_SETTERS_MAP
}

void SettingsManager::SetValue(const DAVA::String& _name, const DAVA::VariantType& _value)
{
	DAVA::List<DAVA::VariantType> additionalArguments;
	SetValue(_name, _value, additionalArguments);
}

DAVA::VariantType SettingsManager::GetValue(const DAVA::String& _name, const DAVA::List<DAVA::VariantType>& additionalArguments)
{
	BEGIN_GETTERS_MAP
		SIMPLE_GETTER(ResourceEditor::SETTINGS_3D_DATA_SOURCEPATH,		DAVA::String("/"))
		SIMPLE_GETTER(ResourceEditor::SETTINGS_PROJECT_PATH,			DAVA::String(""))
		SIMPLE_GETTER(ResourceEditor::SETTINGS_CAMERA_SPEED_INDEX,		0)
		EXTENDED_GETTER(ResourceEditor::SETTINGS_CAMERA_SPEED_VALUE,	ResolveCameraSpeedValueKeyDefault)
		SIMPLE_GETTER(ResourceEditor::SETTINGS_SCREEN_WIDTH,			1024)
		SIMPLE_GETTER(ResourceEditor::SETTINGS_SCREEN_HEIGHT,			690)
		SIMPLE_GETTER(ResourceEditor::SETTINGS_LANGUAGE,				DAVA::String("en"))
		SIMPLE_GETTER(ResourceEditor::SETTINGS_SHOW_OUTPUT,				true)
		SIMPLE_GETTER(ResourceEditor::SETTINGS_LEFT_PANEL_WIDTH,		ControlsFactory::LEFT_PANEL_WIDTH)
		SIMPLE_GETTER(ResourceEditor::SETTINGS_RIGHT_PANEL_WIDTH,		ControlsFactory::RIGHT_PANEL_WIDTH)
		EXTENDED_GETTER(ResourceEditor::SETTINGS_LOD_LAYER,				ResolveLodLevelKeyDefault)
		SIMPLE_GETTER(ResourceEditor::SETTINGS_LAST_OPENED_FILES_COUNT,	0)
		EXTENDED_GETTER(ResourceEditor::SETTINGS_LAST_OPENED_FILE,		ResolveLastOpenedFileKeyDefault)
		SIMPLE_GETTER(ResourceEditor::SETTINGS_DRAW_GRID,				true)
		SIMPLE_GETTER(ResourceEditor::SETTINGS_ENABLE_IMPOSTERS,		true)
		SIMPLE_GETTER(ResourceEditor::SETTINGS_TEXTURE_VIEW_GPU,		GPU_UNKNOWN)
		SIMPLE_GETTER(ResourceEditor::SETTINGS_MATERIALS_AMBIENT,		Color::White)
		SIMPLE_GETTER(ResourceEditor::SETTINGS_MATERIALS_DIFFUSE,		Color::White)
		SIMPLE_GETTER(ResourceEditor::SETTINGS_MATERIALS_SPECULAR,		Color::White)
		SIMPLE_GETTER(ResourceEditor::SETTINGS_DESIGNER_NAME,			DAVA::String("nobody"))
		SIMPLE_GETTER(ResourceEditor::SETTINGS_PREVIEW_DIALOG_ENABLED,	false)
	END_GETTERS_MAP
}

DAVA::VariantType SettingsManager::GetValue(const DAVA::String& _name)
{
	 DAVA::List<DAVA::VariantType> additionalArguments;
	 return GetValue(_name, additionalArguments);
}

FilePath SettingsManager::GetParticlesConfigsPath()
{
	DAVA::String projString = GetValue(ResourceEditor::SETTINGS_PROJECT_PATH).AsString();
	return DAVA::FilePath(projString + "Data/Configs/Particles/");
}

void SettingsManager::ResolveCameraSpeedValueKeyDefault(const DAVA::List<DAVA::VariantType>& additionalArguments,
														DAVA::VariantType& newDefValue, DAVA::String& newKey)
{
	ResolveCameraSpeedValueKey(additionalArguments, DAVA::VariantType(), newKey);
	
	int32 camSpeedIndex = additionalArguments.front().AsInt32();
	static const float32 speedConst[] = {35, 100, 250, 400};
	newDefValue = DAVA::VariantType(speedConst[camSpeedIndex]);
}

void SettingsManager::ResolveCameraSpeedValueKey(const DAVA::List<DAVA::VariantType>& additionalArguments,
												 const DAVA::VariantType& newConfigurationValue,
												 DAVA::String& newKey)
{
	DVASSERT(additionalArguments.size()==1);
	int32 camSpeedIndex = additionalArguments.front().AsInt32();
	DVASSERT(camSpeedIndex >= 0 && camSpeedIndex < 4);
	
	newKey = Format("CameraSpeedValue_%d", camSpeedIndex);
}

void SettingsManager::ResolveLastOpenedFileKeyDefault(const DAVA::List<DAVA::VariantType>& additionalArguments,
													  DAVA::VariantType& newDefValue, DAVA::String& newKey)
{
	ResolveLastOpenedFileKey(additionalArguments, DAVA::VariantType(), newKey);
	newDefValue = DAVA::VariantType("");
}

void SettingsManager::ResolveLastOpenedFileKey(const DAVA::List<DAVA::VariantType>& additionalArguments,
											   const DAVA::VariantType& newConfigurationValue,
											   DAVA::String& newKey)
{
	DVASSERT(additionalArguments.size()==1);
	
	int32 index = additionalArguments.front().AsInt32();
	int32 count = GetValue(ResourceEditor::SETTINGS_LAST_OPENED_FILES_COUNT).AsInt32();
	DVASSERT((0 <= index) && (index < count));
	
	newKey = Format("LastOpenedFile_%d", index);
}

void SettingsManager::AddLastOpenedFile(const DAVA::List<DAVA::VariantType>& additionalArguments, const DAVA::VariantType& newConfigurationValue, DAVA::String& newKey)
{
	FilePath pathToFile(newConfigurationValue.AsString());
	
	DVASSERT(pathToFile.Exists());
	
	Vector<String> filesList;
	int32 count = GetValue(ResourceEditor::SETTINGS_LAST_OPENED_FILES_COUNT).AsInt32();
	for(int32 i = 0; i < count; ++i)
	{
		String path = settings->GetString(Format("LastOpenedFile_%d", i), "");
		if(path != pathToFile.GetAbsolutePathname())
		{
			filesList.push_back(path);
		}
	}
	
	filesList.insert(filesList.begin(), pathToFile.GetAbsolutePathname());
	settings->SetInt32(ResourceEditor::SETTINGS_LAST_OPENED_FILES_COUNT, count);
	Save();
	emit ConfigurationChanged(ResourceEditor::SETTINGS_LAST_OPENED_FILES_COUNT);
	
	count = 0;
	for(;(count < (int32)filesList.size()) && (count < RESENT_FILES_COUNT); ++count)
	{
		settings->SetString(Format("LastOpenedFile_%d", count), filesList[count]);
		Save();
		emit ConfigurationChanged(Format("LastOpenedFile_%d", count));
	}
	newKey = Format("LastOpenedFile_%d", count);
}

void SettingsManager::CameraSpeedIndexCheck(const DAVA::List<DAVA::VariantType>& additionalArguments, const DAVA::VariantType& newConfigurationValue, DAVA::String& newKey)
{
	int32 camSpeedIndex = newConfigurationValue.AsInt32();
	DVASSERT(camSpeedIndex >= 0 && camSpeedIndex < 4);
}

void SettingsManager::ResolveLodLevelKeyDefault(const DAVA::List<DAVA::VariantType>& additionalArguments, DAVA::VariantType& newDefValue, DAVA::String& newKey)
{
	DVASSERT(additionalArguments.size() == 1);
	int32 layerNum = additionalArguments.front().AsInt32();
	DVASSERT(0 <= layerNum && layerNum < LodComponent::MAX_LOD_LAYERS);
	newKey = Format("LODLayer_%d", layerNum);
	newDefValue = DAVA::VariantType(LodComponent::GetDefaultDistance(layerNum));
}

void SettingsManager::ResolveLodLevelKey(const DAVA::List<DAVA::VariantType>& additionalArguments, const DAVA::VariantType& newConfigurationValue, DAVA::String& newKey)
{
	DVASSERT(additionalArguments.size() == 1);
	int32 layerNum = additionalArguments.front().AsInt32();
	DVASSERT(0 <= layerNum && layerNum < LodComponent::MAX_LOD_LAYERS);
	newKey = Format("LODLayer_%d", layerNum);
}

void SettingsManager::SetImposterOption(const DAVA::List<DAVA::VariantType>& additionalArguments, const DAVA::VariantType& newConfigurationValue, DAVA::String& newKey)
{
	RenderManager::Instance()->GetOptions()->SetOption(RenderOptions::IMPOSTERS_ENABLE, newConfigurationValue.AsBool());
}

void SettingsManager::SetMaterialsColor(const DAVA::Color &ambient, const DAVA::Color &diffuse, const DAVA::Color &specular)
{
	SetValue(ResourceEditor::SETTINGS_MATERIALS_AMBIENT, DAVA::VariantType(ambient));
	SetValue(ResourceEditor::SETTINGS_MATERIALS_DIFFUSE, DAVA::VariantType(diffuse));
	SetValue(ResourceEditor::SETTINGS_MATERIALS_SPECULAR, DAVA::VariantType(specular));
}
	
void SettingsManager::Save()
{
    settings->Save(CONFIG_FILE);
}

void SettingsManager::ApplyOptions()
{
    if(RenderManager::Instance())
    {
        RenderManager::Instance()->GetOptions()->SetOption(RenderOptions::IMPOSTERS_ENABLE, settings->GetBool(ResourceEditor::SETTINGS_ENABLE_IMPOSTERS, true));
    }
}

KeyedArchive *SettingsManager::GetSettings()
{
	return settings;
}

float SettingsManager::GetCameraSpeed()
{
	DAVA::List<DAVA::VariantType> arguments;
	arguments.push_back(DAVA::VariantType(0));
	DAVA::int32 index = GetValue(ResourceEditor::SETTINGS_CAMERA_SPEED_INDEX,arguments).AsInt32();
	arguments.clear();
	arguments.push_back(DAVA::VariantType(index));
	return GetValue(ResourceEditor::SETTINGS_CAMERA_SPEED_VALUE,arguments).AsFloat();
};