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


#include "ControlCommands.h"
#include "CommandsController.h"
#include "AlignDistribute/AlignDistributeManager.h"

using namespace DAVA;

ControlsMoveCommand::ControlsMoveCommand(const HierarchyTreeController::SELECTEDCONTROLNODES& controls, const Vector2& delta, bool alignControlsToIntegerPos)
{
	this->controls = controls;
	this->delta = delta;
    this->alignControlsToIntegerPos = alignControlsToIntegerPos;
}

void ControlsMoveCommand::Execute()
{
	ApplyMove(this->delta, this->alignControlsToIntegerPos);
    
    // Notify the Grid some properties were changed.
    CommandsController::Instance()->EmitUpdatePropertyValues();
}

void ControlsMoveCommand::Rollback()
{
	// Return the controls to the previous positions.
	Vector2 prevDelta = -1 * delta;
	ApplyMove(prevDelta, false);

    // Notify the Grid some properties were changed.
    CommandsController::Instance()->EmitUpdatePropertyValues();
}

void ControlsMoveCommand::ApplyMove(const Vector2& moveDelta, bool applyAlign)
{
	for (HierarchyTreeController::SELECTEDCONTROLNODES::iterator iter = this->controls.begin();
         iter != this->controls.end(); iter ++)
    {
        BaseMetadata* baseMetadata = GetMetadataForTreeNode((*iter)->GetId());
		
        // This command is NOT state-aware and contains one and only param.
        baseMetadata->SetActiveParamID(0);
        baseMetadata->ApplyMove(moveDelta, applyAlign);
		
        SAFE_DELETE(baseMetadata);
    }
}

ControlResizeCommand::ControlResizeCommand(HierarchyTreeNode::HIERARCHYTREENODEID nodeId, const Rect& originalRect, const Rect& newRect)
{
	this->nodeId = nodeId;
	this->originalRect = originalRect;
	this->newRect = newRect;
}

void ControlResizeCommand::Execute()
{
	ApplyResize(originalRect, newRect);
	
	// Notify the Grid some properties were changed.
    CommandsController::Instance()->EmitUpdatePropertyValues();
}

void ControlResizeCommand::Rollback()
{
	// Return the controls to the previous size.
	ApplyResize(newRect, originalRect);
	
    // Notify the Grid some properties were changed.
    CommandsController::Instance()->EmitUpdatePropertyValues();
}

void ControlResizeCommand::ApplyResize(const Rect& prevRect, const Rect& updatedRect)
{
	
    BaseMetadata* baseMetadata = GetMetadataForTreeNode(nodeId);

    // This command is NOT state-aware and contains one and only param.
    baseMetadata->SetActiveParamID(0);
    baseMetadata->ApplyResize(prevRect, updatedRect);
    
    SAFE_DELETE(baseMetadata);
}

ControlsAdjustSizeCommand::ControlsAdjustSizeCommand(const HierarchyTreeController::SELECTEDCONTROLNODES& controls)
{
	this->selectedControls = controls;
}

void ControlsAdjustSizeCommand::Execute()
{
	// Apply resize and save previous size data for possible undo action
	this->prevSizeData = ApplyAjustedSize(selectedControls);

    CommandsController::Instance()->EmitUpdatePropertyValues();
}

void ControlsAdjustSizeCommand::Rollback()
{
	UndoAdjustedSize(prevSizeData);

    // Notify the Grid some properties were changed.
    CommandsController::Instance()->EmitUpdatePropertyValues();
}

ControlsPositionData ControlsAdjustSizeCommand::ApplyAjustedSize(HierarchyTreeController::SELECTEDCONTROLNODES& controls)
{
	ControlsPositionData resultData;
	
	for (HierarchyTreeController::SELECTEDCONTROLNODES::iterator iter = controls.begin(); iter != controls.end(); ++iter)
	{
		HierarchyTreeControlNode* control = (*iter);
		UIControl* uiControl = control->GetUIObject();
		int32 nodeId = control->GetId();
		
		if (uiControl)
		{
			// Get sprite
			Sprite* sprite = uiControl->GetSprite();
			Rect prevRect = uiControl->GetRect();
			Rect updatedRect = prevRect;
			
			if (sprite)
			{
				// Save control size data for undo
				resultData.AddControl(uiControl);
				// Set new size of updated rect
				updatedRect.dx = sprite->GetWidth();
				updatedRect.dy = sprite->GetHeight();
			
				BaseMetadata* baseMetadata = GetMetadataForTreeNode(nodeId);

   				// This command is NOT state-aware and contains one and only param.
   				baseMetadata->SetActiveParamID(0);
  		 		baseMetadata->ApplyResize(prevRect, updatedRect);
    
  		  		SAFE_DELETE(baseMetadata);
			}
		}
	}
	
	return resultData;
}
void ControlsAdjustSizeCommand::UndoAdjustedSize(const ControlsPositionData& sizeData)
{
	for (Map<UIControl*, Rect>::const_iterator iter = sizeData.GetControlPositions().begin();
		 iter != sizeData.GetControlPositions().end(); iter ++)
	{
		UIControl* control = iter->first;
		Rect rect = iter->second;

		if (control)
		{
			control->SetRect(rect);
		}
	}
}

ControlsAlignDistributeCommand::ControlsAlignDistributeCommand(const HierarchyTreeController::SELECTEDCONTROLNODES& controls, eAlignControlsType alignType)
{
	this->selectedControls = controls;
	this->selectedAlignType = alignType;
	this->commandMode = MODE_ALIGN;
}

ControlsAlignDistributeCommand::ControlsAlignDistributeCommand(const HierarchyTreeController::SELECTEDCONTROLNODES& controls, eDistributeControlsType distributeType)
{
	this->selectedControls = controls;
	this->selectedDistributeType = distributeType;
	this->commandMode = MODE_DISTRIBUTE;
}

void ControlsAlignDistributeCommand::Execute()
{
	List<UIControl*> selectedUIControls;
	for (HierarchyTreeController::SELECTEDCONTROLNODES::iterator iter = selectedControls.begin(); iter != selectedControls.end(); ++iter)
	{
		HierarchyTreeControlNode* control = (*iter);
		UIControl* uiControl = control->GetUIObject();
		if (uiControl)
		{
			selectedUIControls.push_back(uiControl);
		}
	}

	switch (this->commandMode)
	{
		case MODE_ALIGN:
		{
			this->prevPositionData = AlignDistributeManager::AlignControls(selectedUIControls, selectedAlignType);
			break;
		}

		case MODE_DISTRIBUTE:
		{
			this->prevPositionData = AlignDistributeManager::DistributeControls(selectedUIControls, selectedDistributeType);
			break;
		}
			
		default:
		{
			DVASSERT_MSG(Format("Unknown Command Mode %i", this->commandMode).c_str(), false);
			break;
		}
	}
}

void ControlsAlignDistributeCommand::Rollback()
{
	AlignDistributeManager::UndoAlignDistribute(prevPositionData);
}

ControlRenameCommand::ControlRenameCommand(HierarchyTreeNode::HIERARCHYTREENODEID nodeId, const QString& originalName, const QString& newName)
{
	this->nodeId = nodeId;
	this->originalName = originalName;
	this->newName = newName;
}

void ControlRenameCommand::Execute()
{
	ApplyRename(originalName, newName);
	
	// Notify the Grid some properties were changed.
    CommandsController::Instance()->EmitUpdatePropertyValues();
}

void ControlRenameCommand::Rollback()
{
	// Return the controls to the previous size.
	ApplyRename(newName, originalName);
	
    // Notify the Grid some properties were changed.
    CommandsController::Instance()->EmitUpdatePropertyValues();
}

void ControlRenameCommand::ApplyRename(const QString& prevName, const QString& updatedName)
{
	
    BaseMetadata* baseMetadata = GetMetadataForTreeNode(nodeId);

    // This command is NOT state-aware and contains one and only param.
    baseMetadata->SetActiveParamID(0);
    baseMetadata->ApplyRename(prevName, updatedName);
    
    SAFE_DELETE(baseMetadata);
}

