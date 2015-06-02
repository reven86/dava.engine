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


#include "HierarchyTreeNode.h"
#include <algorithm>

HierarchyTreeNode::HIERARCHYTREENODEID HierarchyTreeNode::nextId = 0;
const QRegExp HierarchyTreeNode::nameRegExp = QRegExp("^[a-zA-Z_][a-zA-Z0-9_]*$");

HierarchyTreeNode::HierarchyTreeNode(const QString& name)
{
	this->id = nextId++;
	this->name = name;
	
	this->redoParentNode = NULL;
	this->redoPreviousNode = NULL;

	marked = false;
	unsavedChangesCounter = 0;
}

HierarchyTreeNode::HierarchyTreeNode(const HierarchyTreeNode* node)
{
	this->id = nextId++;
	this->name = node->name;
	this->extraData = node->extraData;

	this->marked = false;
	unsavedChangesCounter = 0;
}

HierarchyTreeNode::~HierarchyTreeNode()
{
    Cleanup();
}

// Add the node to the list.
void HierarchyTreeNode::AddTreeNode(HierarchyTreeNode* treeNode)
{
    if (treeNode == NULL)
    {
        return;
        // TODO! assert if NULL!
    }

    HIERARCHYTREENODESITER iter = std::find(childNodes.begin(), childNodes.end(), treeNode);
    if (iter == childNodes.end())
    {
        childNodes.push_back(treeNode);
    }

	//childNodes.insert(treeNode);
}

// Add the node to the list after the particular node.
void HierarchyTreeNode::AddTreeNode(HierarchyTreeNode* treeNode, HierarchyTreeNode* nodeToAddAfter)
{
    if (treeNode == NULL)
        return;
	
	HIERARCHYTREENODESITER iter = std::find(childNodes.begin(), childNodes.end(), treeNode);
    if (iter != childNodes.end())
		return;

	if (nodeToAddAfter == NULL ||
        nodeToAddAfter == this )    //YZ: (nodeToAddAfter == this) WTF????
	{
		childNodes.push_front(treeNode);
		return;
	}

	// Look for the "nodeToAddAfter" to insert the tree node after it.
    HIERARCHYTREENODESITER nodeAfterIter = std::find(childNodes.begin(), childNodes.end(), nodeToAddAfter);
    if (nodeAfterIter == childNodes.end())
    {
		return AddTreeNode(treeNode);
    }

	nodeAfterIter ++;
	if (nodeAfterIter == childNodes.end())
    {
		return AddTreeNode(treeNode);
    }
	
	childNodes.insert(nodeAfterIter, treeNode);
}

// Remove the node from the list, return TRUE if succeeded.
bool HierarchyTreeNode::RemoveTreeNode(HierarchyTreeNode* treeNode, bool needDelete, bool needRemoveFromScene)
{
    if (treeNode == NULL)
    {
        // TODO! ASSERT!
        return false;
    }
    
    HIERARCHYTREENODESITER iterToDelete = std::find(childNodes.begin(), childNodes.end(), treeNode);
    if (iterToDelete != childNodes.end())
    {
        //childNodes.remove(treeNode);
		childNodes.erase(iterToDelete);

		if (needRemoveFromScene)
		{
			treeNode->RemoveTreeNodeFromScene();
		}

		if (needDelete)
		{
			delete treeNode;
		}
		
        return true;
    }

    return false;
}

// Access to the nodes list.
const HierarchyTreeNode::HIERARCHYTREENODESLIST& HierarchyTreeNode::GetChildNodes() const
{
    return childNodes;
}

// Cleanup the list of tree nodes.
void HierarchyTreeNode::Cleanup()
{
    for (HIERARCHYTREENODESITER iter = childNodes.begin(); iter != childNodes.end(); ++iter)
    {
        delete *iter;
    }
    
    childNodes.clear();
}

bool HierarchyTreeNode::IsHasChild(const HierarchyTreeNode* node) const
{
	for (HIERARCHYTREENODESLIST::const_iterator iter = childNodes.begin(); iter != childNodes.end(); ++iter)
	{
		const HierarchyTreeNode* item = (*iter);
		if (item == node)
			return true;
		
		if (item->IsHasChild(node))
			return true;
	}
	return false;
}

void HierarchyTreeNode::PrepareRemoveFromSceneInformation()
{
	if (!GetParent())
	{
		this->redoParentNode = NULL;
		this->redoPreviousNode = NULL;
		return;
	}

	this->redoParentNode = GetParent();
	this->redoPreviousNode = NULL;

	HierarchyTreeNode* parentNode = GetParent();
	// Look for the Redo Node in the Children List, and remember the previous one.
	// This is needed to restore the previous node position in case of Redo.
	for (List<HierarchyTreeNode*>::const_iterator iter = parentNode->GetChildNodes().begin();
		 iter != parentNode->GetChildNodes().end(); iter ++)
	{
		if (*iter == this)
        {
            if (iter != parentNode->GetChildNodes().begin())
            {
                iter --;
                this->redoPreviousNode = (*iter);
            }
			break;
		}
	}
}

bool HierarchyTreeNode::IsMarked() const
{
	return marked;
}

bool HierarchyTreeNode::IsNeedSave() const
{
	return IsMarked() | (this->unsavedChangesCounter != 0) | HasUnsavedChilder();
}

bool HierarchyTreeNode::HasUnsavedChilder() const
{
	for (HIERARCHYTREENODESCONSTITER it = childNodes.begin(); it != childNodes.end(); ++it)
	{
		if ((*it)->IsNeedSave())
		{
			return true;
		}
	}

	return false;
}

void HierarchyTreeNode::SetMarked(bool marked)
{
	this->marked = marked;
}

void HierarchyTreeNode::SetChildrenMarked(bool marked, bool recursive)
{
	HIERARCHYTREENODESITER it;
	for (it = childNodes.begin(); it != childNodes.end(); ++it)
	{
		(*it)->SetMarked(marked);

		if (recursive)
		{
			(*it)->SetChildrenMarked(marked, recursive);
		}
	}
}

// Access to the screen unsaved changes counter.
void HierarchyTreeNode::IncrementUnsavedChanges()
{
	unsavedChangesCounter ++;
}

void HierarchyTreeNode::DecrementUnsavedChanges()
{
	unsavedChangesCounter --;
}

void HierarchyTreeNode::ResetUnsavedChanges()
{
	unsavedChangesCounter = 0;
	SetMarked(false);
}

HierarchyTreeNode::AlignData HierarchyTreeNode::SaveAlignData(UIControl* uiControl)
{
	AlignData resultData;
	if (!uiControl)
	{
		return resultData;
	}
	
	resultData.leftAlign = uiControl->GetLeftAlign();
	resultData.hcenterAlign = uiControl->GetHCenterAlign();
	resultData.rightAlign = uiControl->GetRightAlign();
	
	resultData.topAlign = uiControl->GetTopAlign();
	resultData.vcenterAlign = uiControl->GetVCenterAlign();
	resultData.bottomAlign = uiControl->GetBottomAlign();
	
	return resultData;
}

void HierarchyTreeNode::RestoreAlignData(UIControl* uiControl, const HierarchyTreeNode::AlignData& alignData)
{
	if (!uiControl)
	{
		return;
	}

	uiControl->SetLeftAlign(alignData.leftAlign);
	uiControl->SetHCenterAlign(alignData.hcenterAlign);
	uiControl->SetRightAlign(alignData.rightAlign);

	uiControl->SetTopAlign(alignData.topAlign);
	uiControl->SetVCenterAlign(alignData.vcenterAlign);
	uiControl->SetBottomAlign(alignData.bottomAlign);
}

const QRegExp& HierarchyTreeNode::GetNameRegExp()
{
    return nameRegExp;
}

void HierarchyTreeNode::OnScreenScaleChanged()
{
}

void HierarchyTreeNode::OnScreenPositionChanged()
{
}
