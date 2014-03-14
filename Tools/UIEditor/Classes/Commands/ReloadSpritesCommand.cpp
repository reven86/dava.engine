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

#include "ReloadSpritesCommand.h"

#include "TexturePacker/ResourcePacker2D.h"
#include "ResourcesManageHelper.h"
#include "UIControlStateHelper.h"
#include "SpritesHelper.h"
#include "errorsListDialog.h"

ReloadSpritesCommand::ReloadSpritesCommand(const HierarchyTreeNode* node) :
    rootNode(node)
{
}

void ReloadSpritesCommand::Execute()
{
    RepackSprites();
    ReloadSprites();
}

void ReloadSpritesCommand::RepackSprites()
{
	ResourcePacker2D *resPacker = new ResourcePacker2D();
	resPacker->InitFolders(ResourcesManageHelper::GetSpritesDatasourceDirectory().toStdString(),
                           ResourcesManageHelper::GetSpritesDirectory().toStdString());
    
    resPacker->PackResources(GPU_UNKNOWN);
	ShowErrorMessage(resPacker->GetErrors());
    
	SafeDelete(resPacker);
}

void ReloadSpritesCommand::ReloadSprites()
{
    Set<Sprite*> spritesToReload = SpritesHelper::EnumerateSprites(rootNode);
    for (Set<Sprite*>::iterator iter = spritesToReload.begin(); iter != spritesToReload.end(); iter ++)
    {
        (*iter)->Reload();
    }
}

void ReloadSpritesCommand::ShowErrorMessage(const Set<String>& errorsSet)
{
	if (!errorsSet.empty())
	{
		ErrorsListDialog errorsDialog;
		errorsDialog.InitializeErrorsList(errorsSet);
		errorsDialog.exec();
	}
}
