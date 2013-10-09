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



#include "LibraryCommands.h"

#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../Qt/Main/QtMainWindowHandler.h"
#include "../Qt/Main/QtUtils.h"


#include "../SceneEditor/SceneEditorScreenMain.h"


#include "../Collada/ColladaConvert.h"

#include "../SceneEditor/SceneValidator.h"
#include "../SceneEditor/EditorSettings.h"
#include "../CommandLine/TextureDescriptor/TextureDescriptorUtils.h"

#include "Classes/Qt/Scene/SceneHelper.h"

#include "DAVAEngine.h"

using namespace DAVA;

LibraryCommand::LibraryCommand(const DAVA::FilePath &pathname, eCommandType _type, CommandList::eCommandId id)
    : Command(_type, id)
    , filePathname(pathname)
{
}

bool LibraryCommand::CheckExtension(const DAVA::String &extenstionToChecking)
{
	return filePathname.IsEqualToExtension(extenstionToChecking);
}



//Add scene to current tab
CommandAddScene::CommandAddScene(const DAVA::FilePath &pathname)
    : LibraryCommand(pathname, Command::COMMAND_UNDO_REDO, CommandList::ID_COMMAND_ADD_SCENE)
	, entity(NULL)
{
	commandName = "Add Scene";
}

CommandAddScene::~CommandAddScene()
{
	SafeRelease(entity);
}


void CommandAddScene::Execute()
{
	if(entity == NULL)
	{
		DVASSERT(CheckExtension(String(".sc2")) && "Wrong extension");
		entity = SceneDataManager::Instance()->AddScene(filePathname);
	}
	else
	{
		SceneData *sceneData = SceneDataManager::Instance()->SceneGetActive();
		if(NULL != sceneData)
		{
			sceneData->AddSceneNode(entity);
		}
	}
}

void CommandAddScene::Cancel()
{
    DVASSERT(CheckExtension(String(".sc2")) && "Wrong extension");

	SceneData *sceneData = SceneDataManager::Instance()->SceneGetActive();
	if(NULL != sceneData)
	{
		sceneData->RemoveSceneNode(entity);
	}
}


//edit scene at new tab
CommandEditScene::CommandEditScene(const DAVA::FilePath &pathname)
    :   LibraryCommand(pathname, COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_EDIT_SCENE)
{
}


void CommandEditScene::Execute()
{
    DVASSERT(CheckExtension(String(".sc2")) && "Wrong extension");
    
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->AddBodyItem(StringToWString(filePathname.GetFilename()), true);
    }

	EditorSettings::Instance()->AddLastOpenedFile(filePathname);
    SceneDataManager::Instance()->EditActiveScene(filePathname);

	// TODO: mainwindow
    //QtMainWindowHandler::Instance()->ShowStatusBarMessage(filePathname.GetAbsolutePathname());
}


//reload root node at current tab
CommandReloadScene::CommandReloadScene(const DAVA::FilePath &pathname)
    :   LibraryCommand(pathname, COMMAND_UNDO_REDO, CommandList::ID_COMMAND_RELOAD_SCENE)
{
	commandName = "Reload Scene";
}


void CommandReloadScene::Execute()
{
    DVASSERT(CheckExtension(String(".sc2")) && "Wrong extension");
    SceneDataManager::Instance()->ReloadScene(filePathname, filePathname);
}

void CommandReloadScene::Cancel()
{
    DVASSERT(CheckExtension(String(".sc2")) && "Wrong extension");
    //TODO: need code here
}


//reload root node at current tab
CommandReloadEntityFrom::CommandReloadEntityFrom(const DAVA::FilePath &pathname)
:   LibraryCommand(pathname, COMMAND_UNDO_REDO, CommandList::ID_COMMAND_RELOAD_ENTITY_FROM)
{
	commandName = "Reload Entity From";
    fromPathname = FilePath();
}


void CommandReloadEntityFrom::Execute()
{
    DVASSERT(CheckExtension(String(".sc2")) && "Wrong extension");
    
    fromPathname = GetOpenFileName(String("Select Scene File"), filePathname.GetDirectory(), String("Scene File (*.sc2)"));
    if(fromPathname.IsEmpty())
    {
        return;
    }
    
    SceneDataManager::Instance()->ReloadScene(filePathname, fromPathname);
}

void CommandReloadEntityFrom::Cancel()
{
    DVASSERT(CheckExtension(String(".sc2")) && "Wrong extension");
    //TODO: need code here
}



//convert from dae to sc2
CommandConvertScene::CommandConvertScene(const DAVA::FilePath &pathname)
    :   LibraryCommand(pathname, COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_CONVERT_SCENE)
{
}


void CommandConvertScene::Execute()
{
    DVASSERT(CheckExtension(String(".dae")) && "Wrong extension");
//    TextureDescriptorUtils::CreateDescriptorsForFolder(EditorSettings::Instance()->GetDataSourcePath());
    DVASSERT(false)
    
    eColladaErrorCodes code = ConvertDaeToSce(filePathname);
    if(code == COLLADA_OK)
    {
        // load sce to scene object
        FilePath path(filePathname);
        path.ReplaceExtension(".sce");

        Scene * scene = new Scene();
        
        Entity *rootNode = scene->GetRootNode(path);
        scene->AddNode(rootNode);
        
        scene->BakeTransforms();
        
        // Export to *.sc2
        path.ReplaceExtension(".sc2");

		SceneHelper::SaveScene(scene, path);

        SafeRelease(scene);
    }
    else if(code == COLLADA_ERROR_OF_ROOT_NODE)
    {
        ShowErrorDialog(String("Can't convert from DAE. Looks like one of materials has same name as root node."));
    }
    else
    {
        ShowErrorDialog(String("Can't convert from DAE."));
    }
}

