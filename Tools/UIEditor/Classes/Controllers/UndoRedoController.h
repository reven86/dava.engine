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


#ifndef __UIEditor__UndoRedoController__
#define __UIEditor__UndoRedoController__

#include "DAVAEngine.h"
#include "../Commands/BaseCommand.h"

namespace DAVA
{
// Undo/Redo Controller for UI Editor.
class UndoRedoController
{
public:
	// Maximum size of the Undo/Redo stacks.
	static const uint32 MAX_UNDO_REDO_STACK_SIZE;

	UndoRedoController();
	virtual ~UndoRedoController();

	// Add the command to Undo/Redo stack. Should be called before each execution of command.
	void AddCommandToUndoStack(BaseCommand* command);
	
	// Undo the last command, returns TRUE if succeeded.
	bool Undo();
	
	// Redo the last command, returns TRUE if succeeded.
	bool Redo();
	
	// Can we perform Undo/Redo?
	bool CanUndo();
	bool CanRedo();

	// Cleanup the stacks.
	void Cleanup();

	// We need to control saved changes for each screen to determine which of them
	// were changed.
	void IncrementUnsavedChanges(bool forUndoStack);
	void DecrementUnsavedChanges(bool forUndoStack);
	void ResetUnsavedChanges();

protected:

	// Add command to the particular stack, with checking the stack size.
	void AddCommandToStack(Deque<BaseCommand*>& stackToAdd, BaseCommand* command);

	// Cleanup the particular stack.
	void CleanupStack(Deque<BaseCommand*>& stackToCleanup);

    HierarchyTreeNode::HIERARCHYTREENODEID  GetCurrentScreenId();
    
    // Stacks.
    Map<HierarchyTreeNode::HIERARCHYTREENODEID,Deque<BaseCommand*> > undoStacks;
    Map<HierarchyTreeNode::HIERARCHYTREENODEID,Deque<BaseCommand*> > redoStacks;
    
};

};
#endif /* defined(__UIEditor__UndoRedoController__) */
