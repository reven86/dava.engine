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


#ifndef __UIEditor__CommandsController__
#define __UIEditor__CommandsController__

#include <QObject>

#include "Base/Singleton.h"
#include "BaseCommand.h"
#include "UndoRedoController.h"

namespace DAVA {
    
// Commands Controller - singleton which is responsible for executing
// commands and performing Undo/Redo.
class CommandsController : public QObject, public Singleton<CommandsController>
{
    Q_OBJECT
public:
	explicit CommandsController(QObject* parent = NULL);
	
    void ExecuteCommand(BaseCommand* command);
	
	// Undo/Redo functionality.
	bool IsUndoAvailable();
	bool IsRedoAvailable();

	bool Undo();
	bool Redo();

	void CleanupUndoRedoStack();

    // This method is called by commands if some property values are changed and needs to be
    // updated.
    void EmitUpdatePropertyValues();
    
    // These methods are called when property change is succeeded or failed.
    void EmitChangePropertySucceeded(const QString& propertyName);
    void EmitChangePropertyFailed(const QString& propertyName);
	
signals:
    // Called when some command changes the properties, so the values need to be updated.
    void UpdatePropertyValues();
    
    // Called when property change is succeeded/failed.
    void ChangePropertySucceeded(const QString& propertyName);
    void ChangePropertyFailed(const QString& propertyName);
	
	// Called when Undo/Redo availability is changed.
	void UndoRedoAvailabilityChanged();

	// Called when number of unsaved changes is changed
	void UnsavedChangesNumberChanged();

protected slots:
	void OnProjectSaved();
    
protected:

	// Undo/Redo controller.
	UndoRedoController undoRedoController;
};
    
}
#endif /* defined(__UIEditor__CommandsController__) */
