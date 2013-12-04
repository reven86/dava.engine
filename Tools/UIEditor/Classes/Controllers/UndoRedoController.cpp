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




#include "UndoRedoController.h"
using namespace DAVA;

const uint32 UndoRedoController::MAX_UNDO_REDO_STACK_SIZE = 20;

UndoRedoController::UndoRedoController()
{
}

UndoRedoController::~UndoRedoController()
{
	Cleanup();
}

void UndoRedoController::Cleanup()
{
	CleanupStack(undoStack);
	CleanupStack(redoStack);
}

void UndoRedoController::CleanupStack(Deque<BaseCommand*>& stackToCleanup)
{
	while (!stackToCleanup.empty())
	{
		BaseCommand* command = stackToCleanup.front();
		SafeRelease(command);
		stackToCleanup.pop_front();
	}
}

void UndoRedoController::AddCommandToUndoStack(BaseCommand* command)
{
	// Store only the commands which support Undo/Redo.
	if (command->IsUndoRedoSupported() == false)
	{
		return;
	}

	SafeRetain(command);
	AddCommandToStack(undoStack, command);
	
	// Adding the Undo command should cleanup the Redo stack.
	CleanupStack(redoStack);
}

bool UndoRedoController::Undo()
{
	if (!CanUndo())
	{
		return false;
	}
	
	BaseCommand* command = undoStack.front();
	AddCommandToStack(redoStack, command);
	undoStack.pop_front();

	command->ActivateCommandScreen();
	command->Rollback();
	return true;
}

// Redo the last command, returns TRUE if succeeded.
bool UndoRedoController::Redo()
{
	if (!CanRedo())
	{
		return false;
	}
	
	BaseCommand* command = redoStack.front();
	AddCommandToStack(undoStack, command);
	redoStack.pop_front();
	
	command->ActivateCommandScreen();
	command->Execute();
	return true;
}

// Can we perform Undo/Redo?
bool UndoRedoController::CanUndo()
{
	return !undoStack.empty();
}

bool UndoRedoController::CanRedo()
{
	return !redoStack.empty();
}

void UndoRedoController::AddCommandToStack(Deque<BaseCommand*>& stackToAdd, BaseCommand* command)
{
	stackToAdd.push_front(command);
	if (stackToAdd.size() > MAX_UNDO_REDO_STACK_SIZE)
	{
		BaseCommand* commandToRemove = stackToAdd.back();
		SafeRelease(commandToRemove);
		stackToAdd.pop_back();
	}
}

void UndoRedoController::IncrementUnsavedChanges(bool forUndoStack)
{
	Deque<BaseCommand*>& activeStack = forUndoStack ? undoStack : redoStack;
	if (activeStack.empty())
	{
		return;
	}
	
	activeStack.front()->IncrementUnsavedChanges();
}

void UndoRedoController::DecrementUnsavedChanges(bool forUndoStack)
{
	Deque<BaseCommand*>& activeStack = forUndoStack ? undoStack : redoStack;
	if (activeStack.empty())
	{
		return;
	}
	
	activeStack.front()->DecrementUnsavedChanges();
}

void UndoRedoController::ResetUnsavedChanges()
{
	// TODO! IMPLEMENT!
}
