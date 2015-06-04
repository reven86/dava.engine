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


#include "PropertiesGridController.h"
#include "HierarchyTreeController.h"

#include "MetadataFactory.h"
#include "UIControlStateHelper.h"

using namespace DAVA;

PropertiesGridController::PropertiesGridController(QObject* parent) :
    QObject(parent)
{
    this->activeNode = NULL;
    this->activeUIControlStates.push_back(UIControl::STATE_NORMAL);
    ConnectToSignals();
}

void PropertiesGridController::ConnectToSignals()
{
    connect(HierarchyTreeController::Instance(),
            SIGNAL(SelectedTreeItemChanged(const HierarchyTreeNode*)),
            this,
            SLOT(OnSelectedTreeItemChanged(const HierarchyTreeNode*)));

    connect(HierarchyTreeController::Instance(),
            SIGNAL(SelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES&, HierarchyTreeController::eExpandControlType)),
            this,
            SLOT(OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES, HierarchyTreeController::eExpandControlType)));
}

PropertiesGridController::~PropertiesGridController()
{
}

const HierarchyTreeNode* PropertiesGridController::GetActiveTreeNode() const
{
    return this->activeNode;
}

const HierarchyTreeController::SELECTEDCONTROLNODES &PropertiesGridController::GetActiveTreeNodesList() const
{
    return activeNodes;
}

const Vector<UIControl::eControlState> &PropertiesGridController::GetActiveUIControlStates() const
{
	return activeUIControlStates;
}

void PropertiesGridController::SetActiveUIControlStates(const Vector<UIControl::eControlState>& newStates)
{
	bool stateChanged = false;

	if (newStates.size() != activeUIControlStates.size())
	{
		stateChanged = true;
	}
	else
	{
		for (uint32 i = 0; i < activeUIControlStates.size(); ++i)
		{
			stateChanged |= (std::find(newStates.begin(), newStates.end(), activeUIControlStates[i]) ==
							 newStates.end());

			if (stateChanged)
			{
				break;
			}
		}
	}

	if (stateChanged)
	{
		this->activeUIControlStates = newStates;
		emit SelectedUIControlStatesChanged(newStates);
	}
}

void PropertiesGridController::OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES
                                                             &selectedNodes, HierarchyTreeController::eExpandControlType expandType)
{
    if (selectedNodes.size() == 0)
    {
        HandleNothingSelected();
        return;
    }
    
    UpdatePropertiesForUIControlNodeList(selectedNodes);
}

void PropertiesGridController::OnSelectedTreeItemChanged(const HierarchyTreeNode* selectedNode)
{
    UpdatePropertiesForSingleNode(selectedNode);
}

void PropertiesGridController::UpdatePropertiesForSingleNode(const HierarchyTreeNode* selectedNode)
{
    if (selectedNode == NULL)
    {
        // Nothing is selected - this is OK too.
        HandleNothingSelected();
        return;
    }

    this->activeNode = selectedNode;

	this->activeUIControlStates.clear();
    this->activeUIControlStates.push_back(UIControlStateHelper::GetDefaultControlState());
  
    // Properties Grid View is ready to create the appropriate widgets.
    emit PropertiesGridUpdated();
}

void PropertiesGridController::UpdatePropertiesForUIControlNodeList(const HierarchyTreeController::SELECTEDCONTROLNODES
                                                                    &selectedNodes)
{
    if (selectedNodes.empty())
    {
        // Nothing is selected - this is OK too.
        // HandleNothingSelected(); TODO!!! IS THIS CORRECT???
        return;
    }

    this->activeNodes = selectedNodes;
    emit PropertiesGridUpdated();
}

void PropertiesGridController::HandleNothingSelected()
{
    CleanupSelection();
    emit UIControlsDeselected();
}

void PropertiesGridController::CleanupSelection()
{
    this->activeNodes.clear();
    this->activeNode = NULL;
}

void PropertiesGridController::OnSelectedStateChanged(UIControl::eControlState newState)
{
	Vector<UIControl::eControlState> newStates;
	newStates.push_back(newState);

	SetActiveUIControlStates(newStates);
}

void PropertiesGridController::OnSelectedStatesChanged(const Vector<UIControl::eControlState>& newStates)
{
	SetActiveUIControlStates(newStates);
}