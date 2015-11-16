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


#include <QLabel>
#include "AddSwitchEntityDialog.h"
#include "Tools/MimeDataHelper/MimeDataHelper.h"
#include "Tools/SelectPathWidget/SelectEntityPathWidget.h"
#include "Main/mainwindow.h"
#include "Qt/Settings/SettingsManager.h"
#include "Qt/Main/QtUtils.h"
#include "QtTools/ConsoleWidget/PointerSerializer.h"
#include "Classes/Commands2/EntityAddCommand.h"
#include "Commands2/EntityRemoveCommand.h"
#include "SwitchEntityCreator.h"
#include "Project/ProjectManager.h"

#include "ui_BaseAddEntityDialog.h"

AddSwitchEntityDialog::AddSwitchEntityDialog( QWidget* parent)
		:BaseAddEntityDialog(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
{
	setAcceptDrops(true);
	setAttribute( Qt::WA_DeleteOnClose, true );
    FilePath defaultPath(ProjectManager::Instance()->GetDataSourcePath());

    SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
    if (scene)
    {
		FilePath scenePath = scene->GetScenePath();
        if (FileSystem::Instance()->Exists(scenePath))
        {
			defaultPath = scenePath.GetDirectory();
		}
	}
	
	SelectEntityPathWidget* firstWidget = new SelectEntityPathWidget(parent, defaultPath.GetAbsolutePathname(),"");
	SelectEntityPathWidget* secondWidget = new SelectEntityPathWidget(parent, defaultPath.GetAbsolutePathname(),"");
	SelectEntityPathWidget* thirdWidget = new SelectEntityPathWidget(parent, defaultPath.GetAbsolutePathname(),"");

	AddControlToUserContainer(firstWidget, "First Entity:");
	AddControlToUserContainer(secondWidget, "Second Entity:");
	AddControlToUserContainer(thirdWidget, "Third Entity:");

	pathWidgets.push_back(firstWidget);
	pathWidgets.push_back(secondWidget);
	pathWidgets.push_back(thirdWidget);

	propEditor->setVisible(false);
	propEditor->setMinimumHeight(0);
	propEditor->setMaximumSize(propEditor->maximumWidth(), 0);
}

AddSwitchEntityDialog::~AddSwitchEntityDialog()
{
	RemoveAllControlsFromUserContainer();
	Q_FOREACH(SelectEntityPathWidget* widget, pathWidgets)
	{
		delete widget;
	}
}

void AddSwitchEntityDialog::CleanupPathWidgets()
{
	Q_FOREACH(SelectEntityPathWidget* widget, pathWidgets)
	{
		widget->EraseWidget();
	}
}

void AddSwitchEntityDialog::GetPathEntities(DAVA::Vector<DAVA::Entity*>& entities, SceneEditor2* editor)
{
	Q_FOREACH(SelectEntityPathWidget* widget, pathWidgets)
	{
		Entity* entity = widget->GetOutputEntity(editor);
		if(entity)
		{
			entities.push_back(entity);
		}
	}
}

void AddSwitchEntityDialog::accept()
{
	SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
	if( NULL == scene)
	{
		CleanupPathWidgets();
		return;
	}
	
	Vector<Entity*> vector;
	GetPathEntities(vector, scene);
	
	if(vector.empty())
	{
		ShowErrorDialog(ResourceEditor::ADD_SWITCH_NODE_DIALOG_NO_CHILDREN);
		return;
	}
	
	CleanupPathWidgets();

	SwitchEntityCreator creator;

	bool canCreateSwitch = true;

	DAVA::uint32 switchCount = (DAVA::uint32)vector.size(); 
	for(DAVA::uint32 i = 0; i < switchCount; ++i)
	{
		if(creator.HasSwitchComponentsRecursive(vector[i]))
		{
			canCreateSwitch = false;
			Logger::Error("Can't create switch in switch: %s%s", vector[i]->GetName().c_str(),
				PointerSerializer::FromPointer(vector[i]).c_str());
            ShowErrorDialog(ResourceEditor::ADD_SWITCH_NODE_DIALOG_DENY_SRC_SWITCH);
			return;
		}
        if(!creator.HasRenderObjectsRecursive(vector[i]))
        {
            canCreateSwitch = false;
            Logger::Error("Entity '%s' hasn't mesh render objects%s", vector[i]->GetName().c_str(),
				PointerSerializer::FromPointer(vector[i]).c_str());
            ShowErrorDialog(ResourceEditor::ADD_SWITCH_NODE_DIALOG_NO_RENDER_OBJECTS);
            return;
        }
	}

	if(canCreateSwitch)
	{
		scene->BeginBatch("Unite entities into switch entity.");

		for(DAVA::uint32 i = 0; i < switchCount; ++i)
		{
			vector[i]->Retain();
			scene->Exec(new EntityRemoveCommand(vector[i]));
		}

		Entity* switchEntity = creator.CreateSwitchEntity(vector);

		scene->Exec(new EntityAddCommand(switchEntity, scene));

		for(DAVA::uint32 i = 0; i < switchCount; ++i)
		{
			vector[i]->Release();
		}

		scene->EndBatch();
		SafeRelease(switchEntity);
	}
	
	BaseAddEntityDialog::accept();
}

void AddSwitchEntityDialog::reject()
{
	CleanupPathWidgets();
	BaseAddEntityDialog::reject();
}

