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


#include "MetadataFactory.h"

#include "UI/UIButton.h"

#include "UIControlMetadata.h"
#include "UIButtonMetadata.h"

#include "UIStaticTextMetadata.h"
#include "UITextFieldMetadata.h"
#include "UISliderMetadata.h"
#include "UIListMetadata.h"
#include "UIScrollViewMetadata.h"
#include "UISpinnerMetadata.h"
#include "UIAggregatorMetadata.h"
#include "UISwitchMetadata.h"
#include "UIScrollBarMetadata.h"
#include "UIParticlesMetadata.h"
#include "UIWebViewMetadata.h"
#include "UIMovieViewMetadata.h"
#include "UIJoypadMetadata.h"
#include "UI3DViewMetadata.h"
#include "UIListCellMetadata.h"

#include "Custom/GuideMetadata.h"

#include "HierarchyTreePlatformNode.h"
#include "HierarchyTreeScreenNode.h"
#include "HierarchyTreeControlNode.h"

using namespace DAVA;

MetadataFactory::MetadataFactory() :
    Singleton<DAVA::MetadataFactory>()
{
}

MetadataFactory::~MetadataFactory()
{
}

PlatformMetadata* MetadataFactory::GetPlatformMetadata() const
{
    return new PlatformMetadata();
}

ScreenMetadata* MetadataFactory::GetScreenMetadata() const
{
    return new ScreenMetadata();
}

AggregatorMetadata* MetadataFactory::GetAggregatorMetadata() const
{
	return new AggregatorMetadata();
}

BaseMetadata* MetadataFactory::GetMetadataForUIControl(const UIControl* uiControl) const
{
    if (dynamic_cast<const UIStaticText*>(uiControl))
    {
        return new UIStaticTextMetadata();
    }

    if (uiControl->GetClassName() == "UIListCell")
    {
        return new UIListCellMetadata();
    }

    if (dynamic_cast<const UIButton*>(uiControl))
    {
        return new UIButtonMetadata();
    }

    if (dynamic_cast<const UITextField*>(uiControl))
    {
        return new UITextFieldMetadata();
    }
    
	if (dynamic_cast<const UISlider*>(uiControl))
	{
		return new UISliderMetadata();
	}
	
	if (dynamic_cast<const UIList*>(uiControl))
	{
		return new UIListMetadata();
	}
	
	if (dynamic_cast<const UIScrollBar*>(uiControl))
	{
		return new UIScrollBarMetadata();
	}
	
	if (dynamic_cast<const UIScrollView*>(uiControl))
	{
		return new UIScrollViewMetadata();
	}

	if (dynamic_cast<const UISpinner*>(uiControl))
	{
		return new UISpinnerMetadata();
	}
	
	if (dynamic_cast<const UISwitch*>(uiControl))
	{
		return new UISwitchMetadata();
	}

	if (dynamic_cast<const UIAggregatorControl*>(uiControl))
	{
		return new UIAggregatorMetadata();
	}

    if (dynamic_cast<const UIParticles*>(uiControl))
	{
		return new UIParticlesMetadata();
	}

    if (dynamic_cast<const UIWebView*>(uiControl))
	{
		return new UIWebViewMetadata();
	}

    if (dynamic_cast<const UIMovieView*>(uiControl))
	{
		return new UIMovieViewMetadata();
	}

    if (dynamic_cast<const UIJoypad*>(uiControl))
	{
		return new UIJoypadMetadata();
	}

    if (dynamic_cast<const UI3DView*>(uiControl))
    {
        return new UI3DViewMetadata();
    }

    // Add metadata for other Controls here.

    return new UIControlMetadata();
}

BaseMetadata* MetadataFactory::GetMetadataForTreeNode(const HierarchyTreeNode* treeNode) const
{
    if (treeNode == NULL)
    {
        Logger::Error("Attempt to get metadata for Tree Node while no TreeNode specified!");
        return NULL;
    }

    // First try pre-defined nodes.
	if (dynamic_cast<const HierarchyTreeAggregatorNode*>(treeNode))
	{
		return GetAggregatorMetadata();
	}
	
    if (dynamic_cast<const HierarchyTreeScreenNode*>(treeNode))
    {
        return GetScreenMetadata();
    }
    
    if (dynamic_cast<const HierarchyTreePlatformNode*>(treeNode))
    {
        return GetPlatformMetadata();
    }

    // Generic one.
    const HierarchyTreeControlNode* uiControlNode = dynamic_cast<const HierarchyTreeControlNode*>(treeNode);
    if (!uiControlNode)
    {
        Logger::Error("Unknown Hierarchy Tree Node type %s!", typeid(treeNode).name());
        return NULL;
    }
    
    return GetMetadataForUIControl(uiControlNode->GetUIObject());
}

BaseMetadata* MetadataFactory::GetMetadataForTreeNode(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID) const
{
    HierarchyTreeNode* treeNode = HierarchyTreeController::Instance()->GetTree().GetNode(treeNodeID);
    if (treeNode == NULL)
    {
        Logger::Error("Attempt to get metadata for Tree Node while Tree Node does not exist for TreeNodeID %i", treeNodeID);
        return NULL;
    }
    
    return GetMetadataForTreeNode(treeNode);
}

BaseMetadata* MetadataFactory::GetMetadataForTreeNodesList(const HierarchyTreeController::SELECTEDCONTROLNODES& nodesList) const
{
    if (nodesList.empty())
    {
        Logger::Error("MetadataFactory::GetMetadataForTreeNodesList - Nodes List is empty!");
        return NULL;
    }

    // Simplified logic for now. If all the UI Controls attached to the node have the same type, return it.
    // Otherwise return common UIControlMetadata.
    bool allNodesHaveSameUIObjectType = true;
    const char* firstUIControlType = NULL;
    for (HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = nodesList.begin();
         iter != nodesList.end(); iter ++)
    {
        UIControl* attachedControl = (*iter)->GetUIObject();
        if (firstUIControlType == NULL)
        {
            // Remember the first node type.
            firstUIControlType = typeid(*attachedControl).name();
            continue;
        }

        // Compare the current control type with the first one.
        if (strcmp(firstUIControlType, typeid(*attachedControl).name()) != 0)
        {
            allNodesHaveSameUIObjectType = false;
            break;
        }
    }
    
    if (allNodesHaveSameUIObjectType)
    {
        // Since all the nodes have the same UI Object type attached, use the first one.
        return GetMetadataForUIControl((*nodesList.begin())->GetUIObject());
    }
    else
    {
        // Return the metadata for common UIControl.
        UIControl* uiControl = new UIControl();
        BaseMetadata* resultMetadata = GetMetadataForUIControl(uiControl);
        
        uiControl->Release();
        return resultMetadata;
    }
}

BaseMetadata* MetadataFactory::GetCustomMetadata(HierarchyTreeScreenNode* screenNode)
{
    if (!screenNode)
    {
        return NULL;
    }

    const GuideData* activeGuide = screenNode->GetActiveGuide();
    if (!activeGuide)
    {
        return NULL;
    }

    return new GuideMetadata(screenNode);
}
