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


#ifndef __UIEditor__ControlsMoveCommand__
#define __UIEditor__ControlsMoveCommand__

#include "BaseCommand.h"
#include "HierarchyTreeController.h"

#include "AlignDistribute/AlignDistributeEnums.h"
#include "AlignDistribute/ControlsPositionData.h"

using namespace DAVA;

class ControlsMoveCommand: public BaseCommand
{
public:
	ControlsMoveCommand(const HierarchyTreeController::SELECTEDCONTROLNODES& controls, const Vector2& delta, bool alignControlsToIntegerPos);
	virtual void Execute();
	virtual void Rollback();

	virtual bool IsUndoRedoSupported() {return true;};

protected:
	// Apply the actual move.
	void ApplyMove(const Vector2& moveDelta, bool applyAlign);
	
private:
	HierarchyTreeController::SELECTEDCONTROLNODES controls;
	Vector2 delta;
    bool alignControlsToIntegerPos;
};

class ControlResizeCommand: public BaseCommand
{
public:
	ControlResizeCommand(HierarchyTreeNode::HIERARCHYTREENODEID nodeId, const Rect& originalRect, const Rect& newRect);
	
	virtual void Execute();
	virtual void Rollback();
	
	virtual bool IsUndoRedoSupported() {return true;};

protected:
	// Apply the actual resize.
	void ApplyResize(const Rect& prevRect, const Rect& updatedRect);

private:
	HierarchyTreeNode::HIERARCHYTREENODEID nodeId;
	Rect originalRect;
	Rect newRect;
};

// Adjust controls size to fit sizeo of sprite
class ControlsAdjustSizeCommand: public BaseCommand
{
public:
	ControlsAdjustSizeCommand(const HierarchyTreeController::SELECTEDCONTROLNODES& controls);
	
	virtual void Execute();
	virtual void Rollback();
	
	virtual bool IsUndoRedoSupported() {return true;};
	
protected:
	ControlsPositionData ApplyAjustedSize(HierarchyTreeController::SELECTEDCONTROLNODES& controls);
	void UndoAdjustedSize(const ControlsPositionData& sizeData);
	
private:
	// List of selected controls
	HierarchyTreeController::SELECTEDCONTROLNODES selectedControls;
	
	// Adjust size result - needed for Undo.
	ControlsPositionData prevSizeData;
};

// Align/distribute the controls.
class ControlsAlignDistributeCommand : public BaseCommand
{
public:
	ControlsAlignDistributeCommand(const HierarchyTreeController::SELECTEDCONTROLNODES& controls, eAlignControlsType alignType);
	ControlsAlignDistributeCommand(const HierarchyTreeController::SELECTEDCONTROLNODES& controls, eDistributeControlsType distributeType);

	virtual void Execute();
	virtual void Rollback();

	virtual bool IsUndoRedoSupported() {return true;};

protected:
	// Command mode.
	enum eCommandMode
	{
		MODE_ALIGN = 0,
		MODE_DISTRIBUTE
	};

	eCommandMode commandMode;
	
	// List of selected controls and align type.
	HierarchyTreeController::SELECTEDCONTROLNODES selectedControls;
	eAlignControlsType selectedAlignType;
	eDistributeControlsType selectedDistributeType;

	// Alignment/distribution result - needed for Undo.
	ControlsPositionData prevPositionData;
};

class ControlRenameCommand : public BaseCommand
{
public:
	ControlRenameCommand(HierarchyTreeNode::HIERARCHYTREENODEID nodeId, const QString& originalName, const QString& newName);
	
	virtual void Execute();
	virtual void Rollback();
	
	virtual bool IsUndoRedoSupported() {return true;};

protected:
	// Apply the rename of control
	void ApplyRename(const QString& prevName, const QString& updatedName);

private:
	HierarchyTreeNode::HIERARCHYTREENODEID nodeId;
	QString originalName;
	QString newName;
};

#endif /* defined(__UIEditor__ControlsMoveCommand__) */
