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


#include "SpritesHelper.h"
#include "UIControlStateHelper.h"

#include "Models/HierarchyTreePlatformNode.h"
#include "Models/HierarchyTreeScreenNode.h"

Set<Sprite*> SpritesHelper::EnumerateSprites(const HierarchyTreeNode* rootNode)
{
    Set<Sprite*> resultSprites;
    for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator platformIter = rootNode->GetChildNodes().begin(); platformIter != rootNode->GetChildNodes().end(); ++platformIter)
    {
        const HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode*>(*platformIter);
        if (!platformNode)
        {
            continue;
        }
        
        for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator screenIter = platformNode->GetChildNodes().begin(); screenIter != platformNode->GetChildNodes().end(); ++screenIter)
        {
            const HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(*screenIter);
            if (!screenNode)
            {
                continue;
            }
            
            for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator controlIter = screenNode->GetChildNodes().begin(); controlIter != screenNode->GetChildNodes().end(); ++controlIter)
            {
                const HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(*controlIter);
                BuildSpritesListRecursive(controlNode, resultSprites);
            }
        }
    }
    
    return resultSprites;
}

void SpritesHelper::BuildSpritesListRecursive(const HierarchyTreeControlNode* controlNode, Set<Sprite*>& spritesList)
{
    if (!controlNode)
    {
        return;
    }
    
    UIButton* buttonControl = dynamic_cast<UIButton*>(controlNode->GetUIObject());
    if (buttonControl)
    {
        BuildSpritesList(spritesList, buttonControl);
    }
    else if (controlNode->GetUIObject() && controlNode->GetUIObject()->GetSprite())
    {
        spritesList.insert(controlNode->GetUIObject()->GetSprite());
    }

    // Repeat for all children.
    const HierarchyTreeNode::HIERARCHYTREENODESLIST& childNodes = controlNode->GetChildNodes();
    for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = childNodes.begin(); iter != childNodes.end(); iter ++)
    {
        const HierarchyTreeControlNode* childNode = dynamic_cast<const HierarchyTreeControlNode*>(*iter);
        BuildSpritesListRecursive(childNode, spritesList);
    }
}

void SpritesHelper::BuildSpritesList(Set<Sprite*>& spritesList, UIButton* button)
{
    // Button might contain backgrounds and/or texts, check both.
    int32 statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int32 i = 0; i < statesCount; ++i)
    {
        Sprite* buttonSprite = button->GetStateSprite(UIControlStateHelper::GetUIControlState(i));
        if (buttonSprite)
        {
            spritesList.insert(buttonSprite);
        }
    }
}
