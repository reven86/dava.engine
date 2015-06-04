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


#ifndef __UIEditor__MetadataFactory__
#define __UIEditor__MetadataFactory__

#include "MetadataFactory.h"
#include "BaseMetadata.h"

#include "PlatformMetadata.h"
#include "ScreenMetadata.h"
#include "AggregatorMetadata.h"
#include "HierarchyTreeController.h"

#include "Base/Singleton.h"
#include "UI/UIControl.h"

namespace DAVA {
    
// Metadata Factory is responsible for returning metadata for particular DAVA UI class.
class MetadataFactory : public Singleton<MetadataFactory>
{
public:
    MetadataFactory();
    virtual ~MetadataFactory();
    
    // Get the metadata for the UIControl based on its type.
    BaseMetadata* GetMetadataForUIControl(const UIControl* uiControl) const;

    // Get the metadata for the tree node based on its type or ID.
    BaseMetadata* GetMetadataForTreeNode(const HierarchyTreeNode* treeNode) const;
    BaseMetadata* GetMetadataForTreeNode(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID) const;

    // Get the common metadata for list of Hierarchy Tree Control nodes.
    BaseMetadata* GetMetadataForTreeNodesList(const HierarchyTreeController::SELECTEDCONTROLNODES& nodesList) const;
    
    // Get custom (non-UIControl) metadata based on what is selected on the screen
    // (guides only for now).
    BaseMetadata* GetCustomMetadata(HierarchyTreeScreenNode* screenNode);

protected:
    // Get the specific metadata for non-UI Controls.
    PlatformMetadata* GetPlatformMetadata() const;
    ScreenMetadata* GetScreenMetadata() const;
	AggregatorMetadata* GetAggregatorMetadata() const;
};
    
}

#endif /* defined(__UIEditor__MetadataFactory__) */
