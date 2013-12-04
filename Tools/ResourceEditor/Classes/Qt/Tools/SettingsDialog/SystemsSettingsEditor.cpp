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



#include "SystemsSettingsEditor.h"
#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/EditorConfig.h"

#include "Tools/QtPropertyEditor/QtPropertyEditor.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataDavaVariant.h"
#include "Qt/Settings/SettingsStateDialog.h"
#include "Main/mainwindow.h"
 
#include <QHeaderView>

#define ADD_HEADER(headerName) header = AddHeader(headerName);

#define INIT_PROPERTY(propertyName, getter, rowName, handlerName) QtPropertyDataDavaVariant* propertyName = new QtPropertyDataDavaVariant(VariantType(getter));\
	AppendProperty(QString(rowName), propertyName, header);\
	connect(propertyName,SIGNAL(ValueChanged(QtPropertyData::ValueChangeReason)),this, SLOT(handlerName));\
	propertiesMap.push_back(PropertyInfo(propertyName, QVariant(getter)));

#define INIT_PROPERTY_WITH_BTN(propertyName, propertyNameBtn, getter, rowName, handlerName,drawStateMap) QtPropertyDataDavaVariant* propertyName = new QtPropertyDataDavaVariant(VariantType(getter));\
	AppendProperty(QString(rowName), propertyName, header);\
	connect(propertyName,SIGNAL(ValueChanged(QtPropertyData::ValueChangeReason)),this, SLOT(handlerName));\
	propertiesMap.push_back(PropertyInfo(propertyName, QVariant(getter), &drawStateMap));\
	propertyName->SetFlags(QtPropertyData::FLAG_IS_NOT_EDITABLE);\
	QPushButton * propertyNameBtn = CreatePushBtn();\
	propertyName->AddOW(QtPropertyOW(propertyNameBtn));\
	QObject::connect(propertyNameBtn, SIGNAL(clicked()), this, SLOT(ShowDialog()));\
	buttonsMap[propertyNameBtn] = propertiesMap.back();\
	propertyName->AddAllowedValue(DAVA::VariantType(getter), ResolveMapToString(drawStateMap).c_str());

#define ST_COLL_DRAW_NOTHING_NAME			"Draw nothing"
#define ST_COLL_DRAW_OBJECTS_NAME			"Draw objects"
#define ST_COLL_DRAW_OBJECTS_SELECTED_NAME	"Draw objects selected"
#define ST_COLL_DRAW_OBJECTS_RAYTEST_NAME	"Draw objects raytest"
#define ST_COLL_DRAW_LAND_NAME				"Draw land"
#define ST_COLL_DRAW_LAND_RAYTEST_NAME		"Draw land raytest"
#define ST_COLL_DRAW_LAND_COLLISION_NAME	"Draw land collision"
#define ST_COLL_DRAW_ALL_NAME				"Draw all"

#define ST_MODIF_OFF_NAME	"Off"
#define ST_MODIF_MOVE_NAME	"Move"
#define ST_MODIF_ROTATE_NAME "Rotate"
#define ST_MODIF_SCALE_NAME	"Scale"

#define ST_AXIS_NONE_NAME	"None"
#define ST_AXIS_X_NAME		"X"
#define ST_AXIS_Y_NAME		"Y"
#define ST_AXIS_Z_NAME		"Z"
#define ST_AXIS_XY_NAME		"XY"
#define ST_AXIS_XZ_NAME		"XZ"
#define ST_AXIS_YZ_NAME		"YZ"

#define ST_SELDRAW_NOTHING_NAME			"Nothing"
#define ST_SELDRAW_DRAW_SHAPE_NAME		"Draw share"
#define ST_SELDRAW_DRAW_CORNERS_NAME	"Draw corners"
#define ST_SELDRAW_FILL_SHAPE_NAME		"Fill share"
#define ST_SELDRAW_NO_DEEP_TEST_NAME	"No deep test"
#define ST_SELDRAW_ALL_NAME				"All"

#define ST_PIVOT_ENTITY_CENTER_NAME	"Entity center"
#define ST_PIVOT_COMMON_CENTER_NAME	"Common center"

#define HEIGHTMAP_DRAW_RELATIVE_NAME	"Draw relative"
#define HEIGHTMAP_DRAW_AVERAGE_NAME		"Draw average"
#define HEIGHTMAP_DRAW_ABS_DROPPER_NAME	"Draw absolute dropper"
#define HEIGHTMAP_DRAW_DROPPER_NAME		"Dropper"
#define HEIGHTMAP_DRAW_COPY_PASTE_NAME	"Copy paste"

#define GET_SENDER_CONTENT 	QtPropertyDataDavaVariant* sender = dynamic_cast<QtPropertyDataDavaVariant*>(QObject::sender());\
if(!sender){\
	return;\
}\
VariantType senderContent(sender->GetVariantValue());

SystemsSettingsEditor::SystemsSettingsEditor( QWidget* parent)
		:QtPropertyEditor(parent)
{
	sceneEditor = QtMainWindow::Instance()->GetCurrentScene();
	InitializeProperties();
	expandAll();
	resizeColumnToContents(0);
}

SystemsSettingsEditor::~SystemsSettingsEditor()
{
	for( DAVA::List<PropertyInfo>::iterator it= propertiesMap.begin(); it != propertiesMap.end(); ++it)
	{
		delete it->property;
	}
	propertiesMap.clear();

	buttonsMap.clear();
}

void SystemsSettingsEditor::InitializeProperties()
{
	QtPropertyItem *header = NULL;
	
	ADD_HEADER("Collision system settings:");
	collisionSysDrawStateMap[ST_COLL_DRAW_NOTHING_NAME]			= std::make_pair<DAVA::uint32,bool>(ST_COLL_DRAW_NOTHING, false);
	collisionSysDrawStateMap[ST_COLL_DRAW_OBJECTS_NAME]			= std::make_pair<DAVA::uint32,bool>(ST_COLL_DRAW_OBJECTS, false);
	collisionSysDrawStateMap[ST_COLL_DRAW_OBJECTS_SELECTED_NAME]= std::make_pair<DAVA::uint32,bool>(ST_COLL_DRAW_OBJECTS_SELECTED, false);
	collisionSysDrawStateMap[ST_COLL_DRAW_OBJECTS_RAYTEST_NAME]	= std::make_pair<DAVA::uint32,bool>(ST_COLL_DRAW_OBJECTS_RAYTEST, false);
	collisionSysDrawStateMap[ST_COLL_DRAW_LAND_NAME]			= std::make_pair<DAVA::uint32,bool>(ST_COLL_DRAW_LAND, false);
	collisionSysDrawStateMap[ST_COLL_DRAW_LAND_RAYTEST_NAME]	= std::make_pair<DAVA::uint32,bool>(ST_COLL_DRAW_LAND_RAYTEST, false);
	collisionSysDrawStateMap[ST_COLL_DRAW_LAND_COLLISION_NAME]	= std::make_pair<DAVA::uint32,bool>(ST_COLL_DRAW_LAND_COLLISION, false);
	collisionSysDrawStateMap[ST_COLL_DRAW_ALL_NAME]				= std::make_pair<DAVA::uint32,bool>(ST_COLL_DRAW_ALL, false);
	int debugDrawFlags = sceneEditor->collisionSystem->GetDebugDrawFlags();
	InitMapWithFlag(&collisionSysDrawStateMap, debugDrawFlags);
	INIT_PROPERTY_WITH_BTN(collSysDrawMode, collSysDrawModeBtn, debugDrawFlags, "Collision draw mode", HandleCollisionDrawMode(QtPropertyData::ValueChangeReason),collisionSysDrawStateMap);

	ADD_HEADER("Selection system settings:");
	selectionSysDrawStateMap[ST_SELDRAW_NOTHING_NAME]		= std::make_pair<DAVA::uint32,bool>(ST_SELDRAW_NOTHING, false);
	selectionSysDrawStateMap[ST_SELDRAW_DRAW_SHAPE_NAME]	= std::make_pair<DAVA::uint32,bool>(ST_SELDRAW_DRAW_SHAPE, false);
	selectionSysDrawStateMap[ST_SELDRAW_DRAW_CORNERS_NAME]	= std::make_pair<DAVA::uint32,bool>(ST_SELDRAW_DRAW_CORNERS, false);
	selectionSysDrawStateMap[ST_SELDRAW_FILL_SHAPE_NAME]	= std::make_pair<DAVA::uint32,bool>(ST_SELDRAW_FILL_SHAPE, false);
	selectionSysDrawStateMap[ST_SELDRAW_NO_DEEP_TEST_NAME]	= std::make_pair<DAVA::uint32,bool>(ST_SELDRAW_NO_DEEP_TEST, false);
	selectionSysDrawStateMap[ST_SELDRAW_ALL_NAME]			= std::make_pair<DAVA::uint32,bool>(ST_SELDRAW_ALL, false);
	InitMapWithFlag(&selectionSysDrawStateMap, sceneEditor->selectionSystem->GetDrawMode());
	INIT_PROPERTY_WITH_BTN(selectionDrawMode, selectionDrawModeBtn, sceneEditor->selectionSystem->GetDrawMode(), "Draw mode", HandleSelectionDrawMode(QtPropertyData::ValueChangeReason),selectionSysDrawStateMap);

	ADD_HEADER("Grid system settings:");
	INIT_PROPERTY(gridMax, sceneEditor->gridSystem->GetGridMax(), "Grid Max", HandleGridMax(QtPropertyData::ValueChangeReason));
	INIT_PROPERTY(gridStep, sceneEditor->gridSystem->GetGridStep(), "Grid Step", HandleGridStep(QtPropertyData::ValueChangeReason));
}

void SystemsSettingsEditor::RestoreInitialSettings()
{
	for (DAVA::List<PropertyInfo>::iterator it= propertiesMap.begin(); it != propertiesMap.end(); ++it)
	{
		QtPropertyDataDavaVariant* property = it->property;
		STATE_FLAGS_MAP *  map = it->flagsMap;
		if(NULL != map)
		{
			InitMapWithFlag(map, it->defaultValue.toInt());
		}
		this->model()->blockSignals(true);
		property->SetValue(it->defaultValue);
		this->model()->blockSignals(false);
	}
}

void SystemsSettingsEditor::InitMapWithFlag(STATE_FLAGS_MAP* map, DAVA::uint32 value)
{
	bool setAllFalse = false;
	for(STATE_FLAGS_MAP::iterator it = map->begin(); it !=  map->end(); ++it)
	{
		DAVA::uint32 flag = it->second.first;
				
		if(flag == 0)
		{
			if(value==0 )
			{
				SetAllCheckedToFalse(map);
				it->second.second = true;
				break;
			}
			it->second.second = false;
		}
		
		if(flag == std::numeric_limits<DAVA::uint32>::max() )//FF...
		{
			if(flag == value)
			{				
				SetAllCheckedToFalse(map);
				it->second.second = true;
				break;
			}
			it->second.second = false;
			continue;
		}
		it->second.second = flag & value;
	}
}

DAVA::String SystemsSettingsEditor::ResolveMapToString(STATE_FLAGS_MAP& map)
{
	DAVA::String retValue = "";
	
	for(DAVA::Map<DAVA::String,std::pair<DAVA::uint32, bool> >::iterator it = map.begin(); it !=  map.end(); ++it)
	{
		DAVA::String description = it->first;
		bool isChecked = it->second.second;
		
		if(isChecked)
		{
			retValue += ( " " + description);
		}
	}
	return retValue;
}

void SystemsSettingsEditor::SetAllCheckedToFalse(STATE_FLAGS_MAP* map)
{
	for(STATE_FLAGS_MAP::iterator it = map->begin(); it !=  map->end(); ++it)
	{
		it->second.second = false;
	}
}

DAVA::uint32 SystemsSettingsEditor::ResolveMapToUint(STATE_FLAGS_MAP& map)
{
	DAVA::uint32 retValue = 0;
	
	for(STATE_FLAGS_MAP::iterator it = map.begin(); it !=  map.end(); ++it)
	{
		bool isChecked = it->second.second;
		DAVA::uint32 flag = it->second.first;
		
		if(isChecked)
		{
			retValue |= flag;
		}
	}
	return retValue;
}

void SystemsSettingsEditor::ShowDialog()
{
	QPushButton * sender = dynamic_cast<QPushButton *>(QObject::sender());
	DVASSERT(sender);

	QtPropertyDataDavaVariant* propertyToChange = buttonsMap[sender].property;
	STATE_FLAGS_MAP* propFlagsMap = buttonsMap[sender].flagsMap;

	DVASSERT(propertyToChange);
	DVASSERT(propFlagsMap);
	uint32 oldValue  = ResolveMapToUint(*propFlagsMap);
	SettingsStateDialog dialog(propFlagsMap,this);
	dialog.exec();
	
	if(dialog.result() != QDialog::Accepted )
	{
		return;
	}
	uint32 newValue = ResolveMapToUint(*propFlagsMap);
	if(newValue == oldValue)
	{
		return;
	}
	
	propertyToChange->AddAllowedValue(DAVA::VariantType((int32)newValue), ResolveMapToString(*propFlagsMap).c_str());
	propertyToChange->SetVariantValue(DAVA::VariantType((int32)newValue));
	propertyToChange->SetValue(QVariant((int32)newValue));
	
}

QPushButton * SystemsSettingsEditor::CreatePushBtn()
{
	QPushButton *configBtn = new QPushButton(QIcon(":/QtIcons/settings.png"), "");
	configBtn->setIconSize(QSize(12, 12));
	configBtn->setFlat(true);
	return configBtn;
}

void SystemsSettingsEditor::HandleGridMax(QtPropertyData::ValueChangeReason reason)
{
	GET_SENDER_CONTENT
	float32 value = senderContent.AsFloat();
	sceneEditor->gridSystem->SetGridMax(value);
}

void SystemsSettingsEditor::HandleGridStep(QtPropertyData::ValueChangeReason reason)
{
	GET_SENDER_CONTENT
	float32 value = senderContent.AsFloat();
	sceneEditor->gridSystem->SetGridStep(value);
}

void SystemsSettingsEditor::HandleCollisionDrawMode(QtPropertyData::ValueChangeReason reason)
{
	DAVA::uint32 value = ResolveMapToUint(collisionSysDrawStateMap);
	sceneEditor->collisionSystem->SetDrawMode(value);
}

void SystemsSettingsEditor::HandleSelectionDrawMode(QtPropertyData::ValueChangeReason reason)
{
	uint32 value = ResolveMapToUint(selectionSysDrawStateMap);
	sceneEditor->selectionSystem->SetDrawMode(value);
}
