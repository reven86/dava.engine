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


#include "BaseMetadata.h"
#include "HierarchyTreeController.h"

using namespace DAVA;

const Vector2 BaseMetadata::INITIAL_CONTROL_SIZE = Vector2(100, 30);

BaseMetadata::BaseMetadata(QObject *parent) :
    QObject(parent),
    activeParamID(BaseMetadataParams::BaseMetadataID_UNKNOWN),
	activeStateIndex(STATE_INDEX_DEFAULT)
{
}

BaseMetadata::~BaseMetadata()
{
    treeNodeParams.clear();
}

void BaseMetadata::SetupParams(const METADATAPARAMSVECT& params)
{
    CleanupParams();
    treeNodeParams = params;

    // The Active Param needs to be defined explicitely.
    activeParamID = BaseMetadataParams::BaseMetadataID_UNKNOWN;
}

const METADATAPARAMSVECT& BaseMetadata::GetParams() const
{
    return treeNodeParams;
}

void BaseMetadata::CleanupParams()
{
    treeNodeParams.clear();
    activeParamID = BaseMetadataParams::BaseMetadataID_UNKNOWN;
}

// Get the list of Params currently in context.
int BaseMetadata::GetParamsCount() const
{
    return this->treeNodeParams.size();
}

// Get the active Parameter in context.
BaseMetadataParams::METADATAPARAMID BaseMetadata::GetActiveParamID() const
{
    return this->activeParamID;
}

// Set the active Parameter in context.
void BaseMetadata::SetActiveParamID(BaseMetadataParams::METADATAPARAMID paramID)
{
    if (VerifyParamID(paramID))
    {
        this->activeParamID = paramID;
    }
}

bool BaseMetadata::VerifyActiveParamID() const
{
    return VerifyParamID(this->activeParamID);
}

bool BaseMetadata::VerifyParamID(BaseMetadataParams::METADATAPARAMID paramID) const
{
    if (treeNodeParams.empty() || paramID < 0 || paramID >= GetParamsCount())
    {
        return false;
    }

    return true;
}

// Initialize the control(s) attached.
void BaseMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        UIControl* control = this->treeNodeParams[i].GetUIControl();

        control->SetName(controlName);
        control->SetSize(INITIAL_CONTROL_SIZE);
        control->SetPosition(position);

        // Default Flags.
        control->SetInputEnabled(GetInitialInputEnabled());

        control->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    }
}

HierarchyTreeNode::HIERARCHYTREENODEID BaseMetadata::GetActiveTreeNodeID() const
{
    if (VerifyActiveParamID() == false)
    {
        return HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
    }
    
    return treeNodeParams[activeParamID].GetTreeNodeID();
}

UIControl* BaseMetadata::GetActiveUIControl() const
{
    if (VerifyActiveParamID() == false)
    {
        return NULL;
    }
    
    return treeNodeParams[activeParamID].GetUIControl();
}

const Vector<UIControl::eControlState> &BaseMetadata::GetUIControlStates() const
{
    return uiControlStates;
}

void BaseMetadata::SetUIControlStates(const Vector<UIControl::eControlState> &controlStates)
{
    uiControlStates = controlStates;
    ResetActiveStateIndex();
}

HierarchyTreeNode* BaseMetadata::GetTreeNode(BaseMetadataParams::METADATAPARAMID paramID) const
{
    if (VerifyParamID(paramID) == false)
    {
        Logger::Error("Param ID %i is invalid for the Metadata!", paramID);
        return NULL;
    }
    
    HierarchyTreeNode::HIERARCHYTREENODEID nodeID = treeNodeParams[paramID].GetTreeNodeID();
    HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(nodeID);
    if (node == NULL)
    {
        Logger::Error("Unable to found Platform Tree Node for Node ID %i!", nodeID);
        return NULL;
    }
    
    return node;
}

HierarchyTreeNode* BaseMetadata::GetActiveTreeNode() const
{
    return GetTreeNode(GetActiveParamID());
}

UIControl::eControlState BaseMetadata::GetCurrentStateForLocalizedText() const
{
    // For non state-aware controls always return fixed state.
    return UIControl::STATE_NORMAL;
}

bool BaseMetadata::IsStateDirty(UIControl::eControlState controlState)
{
    // If at least one state for the Metadata attached is dirty - return true.
    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        HierarchyTreeNode* treeNode = GetTreeNode(i);
        if (!treeNode)
        {
            continue;
        }
        
        // State is dirty if at least one property exist in the dirty map.
        if (treeNode->GetExtraData().IsStatePropertyDirtyMapEmpty(controlState) == false)
        {
            return true;
        }
    }

    return false;
}

bool BaseMetadata::IsStateDirtyForProperty(UIControl::eControlState controlState, const QString& propertyName)
{
    // If at least one state for the Metadata attached is dirty - return true.
    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        HierarchyTreeNode* treeNode = GetTreeNode(i);
        if (!treeNode)
        {
            continue;
        }

        if (treeNode->GetExtraData().IsStatePropertyDirty(controlState, propertyName))
        {
            return true;
        }
    }
    
    return false;
}

void BaseMetadata::SetStateDirtyForProperty(UIControl::eControlState controlState, const QString& propertyName,
                                            bool value)
{
    // Perform set for all nodes.
    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        HierarchyTreeNode* treeNode = GetTreeNode(i);
        if (!treeNode)
        {
            continue;
        }
        
        treeNode->GetExtraData().SetStatePropertyDirty(controlState, propertyName, value);
    }
}

bool BaseMetadata::IsActiveStateDirtyForProperty(const QString& propertyName)
{
    return IsStateDirtyForProperty(this->uiControlStates[GetActiveStateIndex()], propertyName);
}

void BaseMetadata::SetActiveStateDirtyForProperty(const QString& propertyName, bool value)
{
    SetStateDirtyForProperty(this->uiControlStates[GetActiveStateIndex()], propertyName, value);
}

// Get the "reference" state to compare all other with.
UIControl::eControlState BaseMetadata::GetReferenceState()
{
    return UIControl::STATE_NORMAL;
}

void BaseMetadata::SetActiveStateIndex(int32 index)
{
	if (index >= STATE_INDEX_DEFAULT && index < (int32)GetStatesCount())
	{
		activeStateIndex = index;
		if (GetActiveUIControl())
		{
			GetActiveUIControl()->SetState(uiControlStates[GetActiveStateIndex()]);
		}
	}
	else
	{
		SetActiveStateIndex(STATE_INDEX_DEFAULT);
		Logger::Error("Invalid state index: %d", index);
	}
}

int32 BaseMetadata::GetActiveStateIndex() const
{
	if (activeStateIndex == STATE_INDEX_DEFAULT)
	{
		return DEFAULT_STATE_INDEX_VALUE;
	}

	return activeStateIndex;
}

void BaseMetadata::ResetActiveStateIndex()
{
	SetActiveStateIndex(STATE_INDEX_DEFAULT);
}

uint32 BaseMetadata::GetStatesCount() const
{
	return uiControlStates.size();
}

void BaseMetadata::ApplyRename(const QString& /*originalName*/, const QString& newName)
{
	SetName(newName);
}
