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



#include "GeneralSettingsEditor.h"
#include "SceneEditor/EditorSettings.h"
#include <QHeaderView>
#include "../../Settings/SettingsManager.h"
#include "../StringConstants.h"

#define SETTINGS_LOCALIZATION_SCREEN_WIDTH			"settingsdialog.screenwidth"
#define SETTINGS_LOCALIZATION_SCREEN_HEIGHT			"settingsdialog.screenheight"
#define SETTINGS_LOCALIZATION_LANGUAGE				"settingsdialog.language"
#define SETTINGS_LOCALIZATION_OUTPUT				"settingsdialog.output"
#define SETTINGS_LOCALIZATION_CAMERA_SPEED_1		"settingsdialog.cameraspeed1"
#define SETTINGS_LOCALIZATION_CAMERA_SPEED_2		"settingsdialog.cameraspeed2"
#define SETTINGS_LOCALIZATION_CAMERA_SPEED_3		"settingsdialog.cameraspeed3"
#define SETTINGS_LOCALIZATION_CAMERA_SPEED_4		"settingsdialog.cameraspeed4"
#define SETTINGS_LOCALIZATION_LEFTPANE_WIDTH		"settingsdialog.leftpanelwidth"
#define SETTINGS_LOCALIZATION_RIGHTPANE_WIDTH		"settingsdialog.rightpanelwidth"
#define SETTINGS_LOCALIZATION_DRAW_GRID				"settingsdialog.drawgrid"
#define SETTINGS_LOCALIZATION_IMPOSTERS				"settingsdialog.imposters"
#define SETTINGS_LOCALIZATION_DESIGNER_NAME			"settingsdialog.designername"
#define SETTINGS_LOCALIZATION_PREVIEW_AT_LIBRARY	"Enable Preview at Library"

#define INITIALIZE_PROPERTY_WITH_ARGUMENTS(propertyName, configName, localizationName, argumentsList)\
	QtPropertyDataDavaVariant* propertyName = new QtPropertyDataDavaVariant(VariantType(SettingsManager::Instance()->GetValue(DAVA::String(configName), argumentsList)));\
	AppendProperty(QString((WStringToString(LocalizedString(localizationName))).c_str()),	propertyName, NULL);\
	connect(propertyName,SIGNAL(ValueChanged(QtPropertyData::ValueChangeReason)),this, SLOT(OnValueChanged(QtPropertyData::ValueChangeReason)));\
	propertiesMap[propertyName] = PropertyData(DAVA::String(configName), argumentsList, VariantType(SettingsManager::Instance()->GetValue(DAVA::String(configName), argumentsList)));

#define INITIALIZE_PROPERTY(propertyName, configName, localizationName)\
	QtPropertyDataDavaVariant* propertyName = new QtPropertyDataDavaVariant(VariantType(SettingsManager::Instance()->GetValue(DAVA::String(configName))));\
	AppendProperty(QString((WStringToString(LocalizedString(localizationName))).c_str()),	propertyName, NULL);\
	connect(propertyName,SIGNAL(ValueChanged(QtPropertyData::ValueChangeReason)),this, SLOT(OnValueChanged(QtPropertyData::ValueChangeReason)));\
	propertiesMap[propertyName] = PropertyData(DAVA::String(configName), DAVA::List<DAVA::VariantType>(), VariantType(SettingsManager::Instance()->GetValue(DAVA::String(configName))));


void GeneralSettingsEditor::InitializeProperties()
{
	DAVA::List<DAVA::VariantType> argumentsSpeed0;
	argumentsSpeed0.push_back(VariantType(0));
	DAVA::List<DAVA::VariantType> argumentsSpeed1;
	argumentsSpeed1.push_back(VariantType(1));
	DAVA::List<DAVA::VariantType> argumentsSpeed2;
	argumentsSpeed2.push_back(VariantType(2));
	DAVA::List<DAVA::VariantType> argumentsSpeed3;
	argumentsSpeed3.push_back(VariantType(3));
		
	INITIALIZE_PROPERTY(propertyScreenWidth, ResourceEditor::SETTINGS_SCREEN_WIDTH, SETTINGS_LOCALIZATION_SCREEN_WIDTH)
	INITIALIZE_PROPERTY(propertyScreenHeight,ResourceEditor::SETTINGS_SCREEN_HEIGHT, SETTINGS_LOCALIZATION_SCREEN_HEIGHT)
	INITIALIZE_PROPERTY(propertyLanguage, ResourceEditor::SETTINGS_LANGUAGE, SETTINGS_LOCALIZATION_LANGUAGE)
	INITIALIZE_PROPERTY(propertyOutput, ResourceEditor::SETTINGS_SHOW_OUTPUT, SETTINGS_LOCALIZATION_OUTPUT)
	INITIALIZE_PROPERTY_WITH_ARGUMENTS(propertyCameraSpeed1, ResourceEditor::SETTINGS_CAMERA_SPEED_VALUE, SETTINGS_LOCALIZATION_CAMERA_SPEED_1, argumentsSpeed0)
	INITIALIZE_PROPERTY_WITH_ARGUMENTS(propertyCameraSpeed2, ResourceEditor::SETTINGS_CAMERA_SPEED_VALUE, SETTINGS_LOCALIZATION_CAMERA_SPEED_2, argumentsSpeed1)
	INITIALIZE_PROPERTY_WITH_ARGUMENTS(propertyCameraSpeed3, ResourceEditor::SETTINGS_CAMERA_SPEED_VALUE, SETTINGS_LOCALIZATION_CAMERA_SPEED_3, argumentsSpeed2)
	INITIALIZE_PROPERTY_WITH_ARGUMENTS(propertyCameraSpeed4, ResourceEditor::SETTINGS_CAMERA_SPEED_VALUE, SETTINGS_LOCALIZATION_CAMERA_SPEED_4, argumentsSpeed3)
	INITIALIZE_PROPERTY(propertyLeftpaneWidth, ResourceEditor::SETTINGS_LEFT_PANEL_WIDTH, SETTINGS_LOCALIZATION_LEFTPANE_WIDTH)
	INITIALIZE_PROPERTY(propertyRightpaneWidth, ResourceEditor::SETTINGS_RIGHT_PANEL_WIDTH, SETTINGS_LOCALIZATION_RIGHTPANE_WIDTH)
	INITIALIZE_PROPERTY(propertyDrawGrid, ResourceEditor::SETTINGS_DRAW_GRID, SETTINGS_LOCALIZATION_DRAW_GRID)
	INITIALIZE_PROPERTY(propertyImposters, ResourceEditor::SETTINGS_ENABLE_IMPOSTERS, SETTINGS_LOCALIZATION_IMPOSTERS)
	INITIALIZE_PROPERTY(propertyDesignerName, ResourceEditor::SETTINGS_DESIGNER_NAME, SETTINGS_LOCALIZATION_DESIGNER_NAME)
	INITIALIZE_PROPERTY(propertyPreviewAtLibrary, ResourceEditor::SETTINGS_PREVIEW_DIALOG_ENABLED, SETTINGS_LOCALIZATION_PREVIEW_AT_LIBRARY)
	
	propertyLanguage->AddAllowedValue(DAVA::VariantType(String("en")), "en");
	propertyLanguage->AddAllowedValue(DAVA::VariantType(String("ru")), "ru");
}

void GeneralSettingsEditor::RestoreInitialSettings()
{
	for (DAVA::Map<QtPropertyDataDavaVariant *, PropertyData>::iterator it= propertiesMap.begin(); it != propertiesMap.end(); ++it)
	{
		SettingsManager::Instance()->SetValue(it->second.configName, it->second.initialValue, it->second.argumentList);
	}
}

GeneralSettingsEditor::GeneralSettingsEditor( QWidget* parent)
		:QtPropertyEditor(parent)
{
	InitializeProperties();

	expandAll();

	resizeColumnToContents(0);
}

GeneralSettingsEditor::~GeneralSettingsEditor()
{
	for (DAVA::Map<QtPropertyDataDavaVariant *, PropertyData>::iterator it= propertiesMap.begin(); it != propertiesMap.end(); ++it)
	{
		delete it->first;
	}
	
	propertiesMap.clear();
}

void GeneralSettingsEditor::OnValueChanged(QtPropertyData::ValueChangeReason)
{
	QtPropertyDataDavaVariant* sender = dynamic_cast<QtPropertyDataDavaVariant*>(QObject::sender());
	if(!sender)
	{
		return;
	}
	VariantType senderContent(sender->GetVariantValue());
	
	SettingsManager::Instance()->SetValue(propertiesMap[sender].configName, senderContent, propertiesMap[sender].argumentList);
}

