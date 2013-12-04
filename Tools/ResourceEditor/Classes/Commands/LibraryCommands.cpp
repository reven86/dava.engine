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
        FilePath path = FilePath::CreateWithNewExtension(filePathname, ".sce");

        Scene * scene = new Scene();
        
        Entity *rootNode = scene->GetRootNode(path);
		if(rootNode)
		{
			rootNode = rootNode->Clone();
			scene->AddNode(rootNode);
			rootNode->Release();
		}
        
        scene->BakeTransforms();
        
        // Export to *.sc2
        path.ReplaceExtension(".sc2");
        scene->Save(path);
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

