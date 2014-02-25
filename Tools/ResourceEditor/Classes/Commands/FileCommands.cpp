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



#include "FileCommands.h"

#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorSettings.h"
#include "../SceneEditor/EditorConfig.h"
#include "../SceneEditor/SceneValidator.h"

#include "../Qt/Main/QtUtils.h"
#include "../Qt/Main/QtMainWindowHandler.h"
#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"

#include "Classes/Qt/Scene/SceneHelper.h"

#include <QFileDialog>
#include <QString>

#include "CommandsManager.h"

using namespace DAVA;


//Save
CommandSaveSpecifiedScene::CommandSaveSpecifiedScene(Entity* activeScene, FilePath& filePath)
:	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_SAVE_SPECIFIED_SCENE)
{
	this->activeScene	= activeScene;
	this->filePath		= filePath;
}

void CommandSaveSpecifiedScene::Execute()
{
	if(NULL == activeScene)
	{
		return;
	}

	QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save Scene File"), 
													QString(this->filePath.GetAbsolutePathname().c_str()),
													QString("Scene File (*.sc2)"));
	if(0 < filePath.size())
	{
		FilePath normalizedPathname = PathnameToDAVAStyle(filePath);	
		EditorSettings::Instance()->AddLastOpenedFile(normalizedPathname);

		DVASSERT(activeScene);
		Entity* entityToAdd = activeScene->Clone();
		
		entityToAdd->SetLocalTransform(Matrix4::IDENTITY);

		Scene* sc = new Scene();
		
		uint32 size = entityToAdd->GetChildrenCount();
		KeyedArchive *customProperties = entityToAdd->GetCustomProperties();
		if (customProperties && customProperties->IsKeyExists(String("editor.referenceToOwner")))
		{
			if(!size)
			{
				sc->AddNode(entityToAdd);
			}
			else
			{
				Vector<Entity*> tempV;
				tempV.reserve(size);
				for (uint32 ci = 0; ci < size; ++ci)
				{
					Entity *child = entityToAdd->GetChild(ci);
					child->Retain();
					tempV.push_back(child);
				}
				for (int32 ci = 0; ci < (int32)tempV.size(); ++ci)
				{
					sc->AddNode(tempV[ci]);
					tempV[ci]->Release();
				}
			}
		}
		else
		{
			sc->AddNode(entityToAdd);
		}

        sc->Save(normalizedPathname);

		SafeRelease(entityToAdd); 
		SafeRelease(sc);
	}

	// TODO: mainwindow
	//QtMainWindowHandler::Instance()->RestoreDefaultFocus();
}



