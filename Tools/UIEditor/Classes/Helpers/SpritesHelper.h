//
//  SpritesHelper.h
//  UIEditor
//
//  Created by Yuri Coder on 12/23/13.
//
//

#ifndef __SPRITESHELPER__H__
#define __SPRITESHELPER__H__

#include "DAVAEngine.h"

#include "Models/HierarchyTreeNode.h"
#include "Models/HierarchyTreeControlNode.h"

using namespace DAVA;

class SpritesHelper
{
public:
    // Get the list of the sprites in the Hierarchy Tree.
    static Set<Sprite*> EnumerateSprites(const HierarchyTreeNode* rootNode);
    
    // Pixelize one and all textures (apply the FILTER_NEAREST filter).
    static void ApplyPixelization(Sprite* sprite);
    static void ApplyPixelizationForAllSprites(const HierarchyTreeNode* rootNode);

protected:
    // Build the sprites list to be reloaded in a recursive way.
    static void BuildSpritesListRecursive(const HierarchyTreeControlNode* controlNode, Set<Sprite*>& spritesToReload);
};

#endif /* defined(__SPRITESHELPER__H__) */
