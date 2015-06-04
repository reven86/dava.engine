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


#include "CopyPasteController.h"
#include "PasteCommand.h"
#include "CommandsController.h"
#include "HierarchyTreeAggregatorControlNode.h"
#include "SubcontrolsHelper.h"

using namespace DAVA;

CopyPasteController::CopyPasteController()
{
	Clear();
}

CopyPasteController::~CopyPasteController()
{
	Clear();
}

CopyPasteController::CopyType CopyPasteController::GetCopyType() const
{
	return copyType;
}

void CopyPasteController::Copy(const HierarchyTreeNode::HIERARCHYTREENODESLIST& items)
{
	if (!items.size())
		return;
	
	Clear();
	CopyType curCopy = CopyTypeNone;
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		const HierarchyTreeNode* node = (*iter);
		const HierarchyTreeScreenNode* screen = dynamic_cast<const HierarchyTreeScreenNode* >(node);
		const HierarchyTreePlatformNode* platform = dynamic_cast<const HierarchyTreePlatformNode*>(node);
		const HierarchyTreeAggregatorNode* aggregator = dynamic_cast<const HierarchyTreeAggregatorNode*>(node);

		if (curCopy == CopyTypeNone)
		{
			if (platform)
				curCopy = CopyTypePlatform;
			else if (aggregator)
				curCopy = CopyTypeAggregator;
			else if (screen)
				curCopy = CopyTypeScreen;
		}
		
		HierarchyTreeNode* copy = NULL;
		if (curCopy == CopyTypePlatform && platform)
		{
			copy = new HierarchyTreePlatformNode(NULL, platform);
		}
		else if (curCopy == CopyTypeAggregator && aggregator)
		{
			copy = new HierarchyTreeAggregatorNode(NULL, aggregator, false);
		}
		else if (curCopy == CopyTypeScreen && screen)
		{
			copy = new HierarchyTreeScreenNode(NULL, screen, false);
		}
		
		if (copy)
		{
			this->items.push_back(copy);
		}
	}
	
	if (this->items.size())
		copyType = curCopy;
}

void CopyPasteController::CopyControls(const HierarchyTreeController::SELECTEDCONTROLNODES& items)
{
	if (!items.size())
		return;
	
	Clear();

    // If at least one subcontrol is selected - block copy, see pls DF-2684.
    if (SubcontrolsSelected(items))
    {
        return;
    }

	for (HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		//HierarchyTreeNode* item = (*iter);
		const HierarchyTreeControlNode* control = (*iter);
		if (!control)
			continue;

		if (ControlIsChild(items, control))
			continue;
		
		this->items.push_back(control->CreateControlCopy(NULL));
	}
	if (this->items.size())
		copyType = CopyTypeControl;
}

bool CopyPasteController::ControlIsChild(const HierarchyTreeController::SELECTEDCONTROLNODES& items, const HierarchyTreeControlNode* control) const
{
	bool isChild = false;
	//skip child control when copy parent
	for (HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		const HierarchyTreeControlNode* parent = (*iter);
		if (parent->IsHasChild(control))
		{
			isChild = true;
			break;
		}
	}
	return isChild;
}

void CopyPasteController::Clear()
{
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		HierarchyTreeNode *node = (*iter);
		SAFE_DELETE(node);
	}

	items.clear();
	copyType = CopyTypeNone;
}


void CopyPasteController::Paste(HierarchyTreeNode* parentNode)
{
	if (!parentNode)
		return;
	
	PasteCommand* cmd = new PasteCommand(parentNode, copyType, &items);
	CommandsController::Instance()->ExecuteCommand(cmd);
	SafeRelease(cmd);
}

bool CopyPasteController::SubcontrolsSelected(const HierarchyTreeController::SELECTEDCONTROLNODES& items)
{
    for (HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = items.begin(); iter != items.end(); ++iter)
    {
        HierarchyTreeControlNode* control = (*iter);
        if (control && SubcontrolsHelper::ControlIsSubcontrol(control->GetUIObject()))
        {
            return true;
        }
    }

    return false;
}
