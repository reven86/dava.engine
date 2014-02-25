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

#include "LandscapeDialog.h"
#include <QLabel>
#include <QMessageBox>
#include "ui_BaseAddEntityDialog.h"
#include "Qt/Settings/SettingsManager.h"
#include "Project/ProjectManager.h"
#include "Main/mainwindow.h"
#include "Commands2/EntityAddCommand.h"
#include "Commands2/EntityRemoveCommand.h"
#include "Commands2/LandscapeSetTexturesCommands.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataDavaVariant.h"
#include "Main/QtUtils.h"
#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"
#include "MaterialEditor/MaterialEditor.h"

#define HEIGHTMAP_TITLE "Open height map"
#define HEIGHTMAP_FILTER "All (*.heightmap *.png);;PNG (*.png);;Height map (*.heightmap) "
#define HEIGHT_MAP_ID 0xABCD

#define CREATE_TITLE "Create"
#define DELETE_TITLE "Delete"
#define OPEN_MAT_EDITOR_TITLE "Edit material"

#define DEFAULT_LANDSCAPE_SIDE_LENGTH	600.0f
#define DEFAULT_LANDSCAPE_HEIGHT		50.0f

const int32 LandscapeDialog::landscapeRelatedCommandIDs[] = {
	CMDID_ENTITY_ADD,
	CMDID_ENTITY_REMOVE,
	CMDID_LANDSCAPE_SET_HEIGHTMAP
};

LandscapeDialog::LandscapeDialog(Entity* _landscapeEntity,  QWidget* parent)
:BaseAddEntityDialog(parent)
{
	setWindowTitle("Landscape Settings");
	setAcceptDrops(true);
	setAttribute( Qt::WA_DeleteOnClose, true );

	SetEntity(_landscapeEntity);
	innerLandscape = DAVA::GetLandscape(entity);
	connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(SceneActivated(SceneEditor2 *)));

	DAVA::List<DAVA::String> heightMapFormats;
	heightMapFormats.push_back(".heightmap");
	
	SelectPathWidgetBase* heightMapWidget = InitPathWidget(parent, HEIGHT_MAP_ID, heightMapFormats);
	
	AddControlToUserContainer(heightMapWidget, "Height map:");
	
	QString btnTitle = CREATE_TITLE;
	bool isLandscapePresent = innerLandscape != NULL;
	if(isLandscapePresent)
	{
		btnTitle = DELETE_TITLE;
	}
	actionButton = new QPushButton(btnTitle, this);
	connect(actionButton, SIGNAL(clicked()), this, SLOT(ActionButtonClicked()));
	AddButton(actionButton);

	openMaterEditorBtn = new QPushButton(OPEN_MAT_EDITOR_TITLE, this);
	connect(openMaterEditorBtn, SIGNAL(clicked()), this, SLOT(MaterialEditorButtonClicked()));
	AddButton(openMaterEditorBtn, 1);
	openMaterEditorBtn->setVisible(isLandscapePresent);

	landscapeSize = Vector3(DEFAULT_LANDSCAPE_SIDE_LENGTH, DEFAULT_LANDSCAPE_SIDE_LENGTH, DEFAULT_LANDSCAPE_HEIGHT);
}

LandscapeDialog::~LandscapeDialog()
{
	disconnect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(SceneActivated(SceneEditor2 *)));
	
	for (DAVA::Map<SelectPathWidgetBase*,int32>::iterator it = widgetMap.begin(); it != widgetMap.end(); ++it)
	{
		delete it->first;
	}
	delete openMaterEditorBtn;
	delete actionButton;
}

SelectPathWidgetBase* LandscapeDialog::InitPathWidget(QWidget* parent, int32 widgetNum, const DAVA::List<DAVA::String>& formats)
{
	bool isHeightMap = widgetNum == HEIGHT_MAP_ID;
	String widgetTitle = HEIGHTMAP_TITLE;
	String fileFilter = HEIGHTMAP_FILTER;

	DAVA::String resFolder = FilePath(ProjectManager::Instance()->CurProjectDataSourcePath().toStdString()).GetAbsolutePathname();
	SelectPathWidgetBase* widget = new SelectPathWidgetBase(parent, true, resFolder,"", widgetTitle, fileFilter);

	if(innerLandscape)
	{
		FilePath widgetFile = isHeightMap ? innerLandscape->GetHeightmapPathname() :
		innerLandscape->GetTextureName((Landscape::eTextureLevel)widgetNum);
		widget->setText(widgetFile.GetAbsolutePathname());
	}
	widgetMap[widget] = widgetNum;
	widget->setEnabled(innerLandscape);
	widget->SetAllowedFormatsList(formats);
	connect(widget, SIGNAL(PathSelected(DAVA::String)), this, SLOT(PathWidgetValueChanged(DAVA::String)));
	return widget;
}

void LandscapeDialog::FillPropertyEditorWithContent()
{
	propEditor->RemovePropertyAll();
	if(NULL == entity)
	{
		return;
	}
	
	landscapeSize = GetSizeOfCurrentLandscape();
	AddMetaObject(&landscapeSize.x, DAVA::MetaInfo::Instance<float>(), "Size");
	AddMetaObject(&landscapeSize.z, DAVA::MetaInfo::Instance<float>(), "Height");
}

void LandscapeDialog::FillWidgetsWithContent()
{
	Landscape*	landscapeToProcess = DAVA::GetLandscape(entity);
	
	for (DAVA::Map<SelectPathWidgetBase*,int32>::iterator it = widgetMap.begin(); it != widgetMap.end(); ++it)
	{
		SelectPathWidgetBase* widget = it->first;
		int32 info = it->second;

		widget->blockSignals(true);
		
		DAVA::String widgetText("");
		if(landscapeToProcess)
		{
			widgetText = (info == HEIGHT_MAP_ID) ? landscapeToProcess->GetHeightmapPathname().GetAbsolutePathname():
				landscapeToProcess->GetTextureName((Landscape::eTextureLevel)info).GetAbsolutePathname();
		}

		widget->setText(widgetText);
		
		widget->blockSignals(false);
		
		widget->setEnabled(landscapeToProcess);
	}
	
	if(landscapeToProcess != NULL)
	{
		actionButton->setText(DELETE_TITLE);
	}
	else
	{
		actionButton->setText(CREATE_TITLE);
	}
	
	openMaterEditorBtn->setVisible( innerLandscape != NULL);
}

void LandscapeDialog::showEvent ( QShowEvent * event )
{
	BaseAddEntityDialog::showEvent(event);
	SetLandscapeEntity(entity);
}

void LandscapeDialog::CommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo)
{
    BaseAddEntityDialog::CommandExecuted(scene, command, redo);
    
	if(!IsLandscapeAffectedByCommand(command))
	{
		return;
	}
	
	int id = command->GetId();
	if( id == CMDID_ENTITY_ADD || id == CMDID_ENTITY_REMOVE)
	{
		Entity* commandEntity = command->GetEntity();
		
		bool isAddEntityCommand = id == CMDID_ENTITY_ADD;
		
		if((isAddEntityCommand && !redo)||(!isAddEntityCommand && redo))
		{
			SetLandscapeEntity(NULL);
		}
		else if((isAddEntityCommand && redo)||(!isAddEntityCommand && !redo))
		{
			SetLandscapeEntity(commandEntity);
		}
	}
	else if(id == CMDID_LANDSCAPE_SET_HEIGHTMAP )
	{
		FillWidgetsWithContent();
	}
}


bool LandscapeDialog::IsLandscapeAffectedByCommand(const Command2* command)
{
	int id = command->GetId();
	for (int32 i = 0; i < COUNT_OF(landscapeRelatedCommandIDs); ++i)
	{
		if (landscapeRelatedCommandIDs[i] != id)
		{
			continue;
		}

		Entity* commandEntity = command->GetEntity();
		//check if it's landscapeEntity for case of addition from library
		return (NULL != FindLandscape(commandEntity));
	}

	return false;
}

void LandscapeDialog::ActionButtonClicked()
{
	SceneEditor2* sceneEditor = QtMainWindow::Instance()->GetCurrentScene();
	bool needToCreate = actionButton->text() == CREATE_TITLE;
	if(needToCreate)
	{
		actionButton->setText(DELETE_TITLE);
		
		Entity* entityToProcess = new Entity();
		entityToProcess->SetName(ResourceEditor::LANDSCAPE_NODE_NAME);
        entityToProcess->SetLocked(true);
		Landscape* newLandscape = new Landscape();
		
		//for(uint32 i = Landscape::TEXTURE_COLOR; i < Landscape::TEXTURE_COUNT; ++i)
		//{
		//	Texture* pinkTexture = Texture::CreatePink();
		//	newLandscape->SetTexture((Landscape::eTextureLevel)i, pinkTexture);
		//	SafeRelease(pinkTexture);
		//}
		RenderComponent* component = new RenderComponent(ScopedPtr<Landscape>(newLandscape));
		entityToProcess->AddComponent(component);

		AABBox3 bboxForLandscape;
		bboxForLandscape.AddPoint(Vector3(-landscapeSize.x/2.f, -landscapeSize.y/2.f, 0.f));
		bboxForLandscape.AddPoint(Vector3(landscapeSize.x/2.f, landscapeSize.y/2.f, landscapeSize.z));
		newLandscape->BuildLandscapeFromHeightmapImage("", bboxForLandscape);

		EntityAddCommand* commandAdd = new EntityAddCommand(entityToProcess, sceneEditor);
		sceneEditor->Exec(commandAdd);
		sceneEditor->selectionSystem->SetSelection(entityToProcess);

		SetLandscapeEntity(entityToProcess);//BaseAddEntityDialog::entity is inited
		SafeRelease(entityToProcess);
	}
	else
	{
		actionButton->setText(CREATE_TITLE);
		
		if(entity != NULL)
		{
			EntityRemoveCommand * command = new EntityRemoveCommand(entity);
			sceneEditor->Exec(command);
		}

		SetLandscapeEntity(NULL);
	}
}

void LandscapeDialog::MaterialEditorButtonClicked()
{
	MaterialEditor::Instance()->show();
}

void LandscapeDialog::OnItemEdited(const QModelIndex &index)
{
	QtPropertyData *data = propEditor->GetProperty(index);
	QString name = data->GetName();

	SceneEditor2 *curScene = QtMainWindow::Instance()->GetCurrentScene();
	if(NULL == curScene)
	{
		return;		
	}

	bool isMultiple = "Size" == name || "Height" == name;
	
	if(isMultiple)
	{
        float val = Max(0.f, data->GetValue().toFloat());
        data->SetValue(QVariant(val));
		curScene->BeginBatch("Landscape resizing");
	}
	
	Command2 *command = (Command2 *) data->CreateLastCommand();
	if(NULL != command)
	{
		curScene->Exec(command);
	}
	
	if("Size" == name || "Height" == name)
	{
		landscapeSize.y = landscapeSize.x;
		AABBox3 bboxForLandscape;
		bboxForLandscape.AddPoint(Vector3(-landscapeSize.x/2.f, -landscapeSize.y/2.f, 0.f));
		bboxForLandscape.AddPoint(Vector3(landscapeSize.x/2.f, landscapeSize.y/2.f, landscapeSize.z));
		LandscapeSetHeightMapCommand* command = new LandscapeSetHeightMapCommand(entity,  innerLandscape->GetHeightmapPathname(), bboxForLandscape);
		curScene->Exec(command);
		
		curScene->EndBatch();
	}
}

Vector3 LandscapeDialog::GetSizeOfCurrentLandscape()
{
	Vector3 retValue;
	if(NULL == innerLandscape)
	{
		return retValue;
	}
	DAVA::AABBox3 bbox = innerLandscape->GetBoundingBox();
	if(!bbox.GetSize().IsZero())
	{
		DAVA::AABBox3 transformedBox = bbox;
		if(NULL != innerLandscape->GetWorldTransformPtr())
		{
			bbox.GetTransformedBox(*innerLandscape->GetWorldTransformPtr(), transformedBox);
		}
		retValue = transformedBox.max - transformedBox.min;
	}
	return retValue;
}

void LandscapeDialog::SetLandscapeEntity(Entity* _landscapeEntity)
{
	SetEntity(_landscapeEntity);
	innerLandscape = DAVA::GetLandscape(_landscapeEntity);
	FillUIbyLandscapeEntity();
}

void LandscapeDialog::FillUIbyLandscapeEntity()
{
	FillWidgetsWithContent();
	FillPropertyEditorWithContent();
	propEditor->expandAll();
	PerformResize();
}

void LandscapeDialog::CleanupPathWidgets()
{
	for (DAVA::Map<SelectPathWidgetBase*,int32>::iterator it = widgetMap.begin(); it != widgetMap.end(); ++it)
	{
		it->first->EraseWidget();
	}
}

void LandscapeDialog::PathWidgetValueChanged(String fileName)
{
	SceneEditor2 *sceneEditor = QtMainWindow::Instance()->GetCurrentScene();
	if(!innerLandscape)
	{
		return;
	}
	SelectPathWidgetBase* sender = dynamic_cast<SelectPathWidgetBase*>(QObject::sender());
	
	FilePath filePath(fileName);
	if(widgetMap[sender] == HEIGHT_MAP_ID)
	{
		FilePath presentPath = innerLandscape->GetHeightmapPathname();
//		if(filePath != presentPath && filePath.Exists())
		if(filePath != presentPath)
		{
			if(filePath.IsEqualToExtension(".png"))
			{
				Vector<Image *> imageVector = ImageLoader::CreateFromFileByExtension(filePath);
				DVASSERT(imageVector.size());
			
				PixelFormat format = imageVector[0]->GetPixelFormat();
				Q_FOREACH(Image* image, imageVector)
				{
					SafeRelease(image);
				}
				
				if( !(format == FORMAT_A8 ||format == FORMAT_A16))
				{
					sender->EraseWidget();
					ShowErrorDialog(ResourceEditor::LANDSCAPE_DIALOG_WRONG_PNG_ERROR);
					return;
				}
			}
			
			LandscapeSetHeightMapCommand* command = new LandscapeSetHeightMapCommand(entity, filePath,
																					 innerLandscape->GetBoundingBox());
			sceneEditor->Exec(command);
		}
	}
}

void LandscapeDialog::SceneActivated(SceneEditor2 *editor)
{
	SetLandscapeEntity(FindLandscapeEntity(editor));
}
