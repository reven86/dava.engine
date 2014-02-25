//
//  SpritesHelper.cpp
//  UIEditor
//
//  Created by Yuri Coder on 12/23/13.
//
//

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

void SpritesHelper::BuildSpritesListRecursive(const HierarchyTreeControlNode* controlNode, Set<Sprite*>& sprites)
{
    if (!controlNode)
    {
        return;
    }
    
    // Specific check for UIButton - it has more than one sprite to reload.
    UIButton* buttonControl = dynamic_cast<UIButton*>(controlNode->GetUIObject());
    if (buttonControl)
    {
        //States cycle for values
        int32 statesCount = UIControlStateHelper::GetUIControlStatesCount();
		for (int32 i = 0; i < statesCount; ++i)
		{
            Sprite* buttonSprite = buttonControl->GetStateSprite(UIControlStateHelper::GetUIControlState(i));
            if (buttonSprite)
            {
                sprites.insert(buttonSprite);
            }
        }
    }
    else if (controlNode->GetUIObject() && controlNode->GetUIObject()->GetSprite())
    {
        sprites.insert(controlNode->GetUIObject()->GetSprite());
    }
    
    // Repeat for all children.
    const HierarchyTreeNode::HIERARCHYTREENODESLIST& childNodes = controlNode->GetChildNodes();
    for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = childNodes.begin(); iter != childNodes.end(); iter ++)
    {
        const HierarchyTreeControlNode* childNode = dynamic_cast<const HierarchyTreeControlNode*>(*iter);
        BuildSpritesListRecursive(childNode, sprites);
    }
}

void SpritesHelper::ApplyPixelization(Sprite* sprite)
{
    if (!sprite)
    {
        return;
    }

    int32 frameCount = sprite->GetFrameCount();
    for (int32 i = 0; i < frameCount; i ++)
    {
        Texture* texture = sprite->GetTexture(i);
        if (!texture || !texture->GetDescriptor())
        {
            continue;
        }

        texture->GeneratePixelesation();
    }
}

void SpritesHelper::ApplyPixelizationForAllSprites(const HierarchyTreeNode* rootNode)
{
    Set<Sprite*> sprites = EnumerateSprites(rootNode);
    for (Set<Sprite*>::iterator iter = sprites.begin(); iter != sprites.end(); iter ++)
    {
        ApplyPixelization(*iter);
    }
}
