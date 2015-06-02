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


#include "ui_BaseAddEntityDialog.h"

#include "Scene3D/Systems/SkyboxSystem.h"
#include "Render/Highlevel/SkyboxRenderObject.h"
#include "CubemapEditor/CubemapUtils.h"
#include "AddSkyboxDialog.h"
#include "Scene/SceneSignals.h"
#include "Classes/Commands2/EntityAddCommand.h"
#include "Classes/Commands2/EntityRemoveCommand.h"

AddSkyboxDialog::AddSkyboxDialog(QWidget* parent) 
	: BaseAddEntityDialog(parent)
	, editorScene(NULL)
{
	controlButton = new QPushButton(this);
	
	AddButton(controlButton);

	connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(OnSceneActivated(SceneEditor2 *)));
}

AddSkyboxDialog::~AddSkyboxDialog()
{
	SafeRelease(editorScene);
}

void AddSkyboxDialog::FillPropertyEditorWithContent()
{
    if(propEditor == NULL)
        return;
    
	propEditor->RemovePropertyAll();
	if(NULL == entity)
		return;

    AddInspMemberToEditor(entity, entity->GetTypeInfo()->Member("name"));

    SkyboxRenderObject *skyBox = GetSkybox(entity);
    if(skyBox)
    {
        AddInspMemberToEditor( skyBox, skyBox->GetTypeInfo()->Member("texture"));
        AddInspMemberToEditor( skyBox, skyBox->GetTypeInfo()->Member("verticalOffset"));
    }
}

void AddSkyboxDialog::OnCreateButtonClicked()
{
	if(editorScene)
	{
		Entity* skyboxEntity = editorScene->skyboxSystem->AddSkybox();
        skyboxEntity->Retain();

		skyboxEntity->GetParent()->RemoveNode(skyboxEntity);
        editorScene->Exec(new EntityAddCommand(skyboxEntity, editorScene)); //To enable Ctrl+Z operation
        skyboxEntity->Release();
	}
}

void  AddSkyboxDialog::OnDeleteButtonClicked()
{
	Entity* skyboxEntity = GetEntity();
	if(skyboxEntity && editorScene)
	{
        editorScene->Exec(new EntityRemoveCommand(skyboxEntity));
	}
}

void AddSkyboxDialog::MakeCreateButton()
{
	QObject::disconnect(controlButton, SIGNAL(clicked()), this, SLOT(OnDeleteButtonClicked()));
	QObject::connect(controlButton, SIGNAL(clicked()), this, SLOT(OnCreateButtonClicked()));
	controlButton->setText("Create");	
}

void AddSkyboxDialog::MakeDeleteButton()
{
	QObject::connect(controlButton, SIGNAL(clicked()), this, SLOT(OnDeleteButtonClicked()));
	QObject::disconnect(controlButton, SIGNAL(clicked()), this, SLOT(OnCreateButtonClicked()));
	controlButton->setText("Delete");	
}

void AddSkyboxDialog::SetEntity(DAVA::Entity* newEntity)
{
    BaseAddEntityDialog::SetEntity(newEntity);
    
    if(entity)
    {
        MakeDeleteButton();
    }
    else
    {
        MakeCreateButton();
    }
    
    FillPropertyEditorWithContent();
}

void AddSkyboxDialog::Show(QWidget* parent, SceneEditor2* scene)
{
	if(NULL != scene)	
	{
		Entity* currentSkybox = scene->skyboxSystem->GetSkyboxEntity();
	
		AddSkyboxDialog* dlg = new AddSkyboxDialog(parent);
		dlg->setAttribute(Qt::WA_DeleteOnClose, true);
		dlg->SetEditorScene(scene);
		dlg->SetEntity(currentSkybox);
		dlg->setWindowTitle("Set up Skybox");
		dlg->show();
	}
}

void AddSkyboxDialog::SetEditorScene(SceneEditor2* scene)
{
	SafeRelease(editorScene);
	editorScene = SafeRetain(scene);
}

SceneEditor2* AddSkyboxDialog::GetEditorScene() const
{
	return editorScene;
}

void AddSkyboxDialog::OnSceneActivated(SceneEditor2 *sceneEditor)
{
    SetEditorScene(sceneEditor);
    SetEntity(sceneEditor->skyboxSystem->GetSkyboxEntity());
}

void AddSkyboxDialog::CommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo)
{
    BaseAddEntityDialog::CommandExecuted(scene, command, redo);
    
    if(NULL == GetSkybox(command->GetEntity()))
    {
        return;
    }

    int id = command->GetId();
    if((id == CMDID_ENTITY_ADD && redo) || (id == CMDID_ENTITY_REMOVE && !redo))
    {
        SetEntity(command->GetEntity());
    }
    else if((id == CMDID_ENTITY_ADD && !redo) || (id == CMDID_ENTITY_REMOVE && redo))
    {
        SetEntity(NULL);
    }
}