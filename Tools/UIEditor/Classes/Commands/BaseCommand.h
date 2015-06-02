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


#ifndef __UIEditor__BaseCommand__
#define __UIEditor__BaseCommand__

#include <DAVAEngine.h>
#include "BaseMetadata.h"
#include "HierarchyTreeController.h"

#include <QMetaProperty>
//Register DAVA::Font type here so we can use it as Qt object
Q_DECLARE_METATYPE(Font*);

namespace DAVA {

// Base Command for all the Commands existing in the UI Editor system.
class BaseCommand: public BaseObject
{
public:
    BaseCommand();
    virtual ~BaseCommand();

    // Execute command.
    virtual void Execute() = 0;

    // Rollback command (only for the commands which does support Undo/Redo).
    virtual void Rollback() {};
	
	// Is the Undo/Redo supported for this command?
	virtual bool IsUndoRedoSupported() = 0;
	
	virtual void ActivateCommandScreen();

	// Access to the screen unsaved changes counter.
	virtual void IncrementUnsavedChanges();
	virtual void DecrementUnsavedChanges();
	void ResetUnsavedChanges();

protected:
    // Get the Metadata for the tree node passed.
    BaseMetadata* GetMetadataForTreeNode(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID);
	
	HierarchyTreeNode::HIERARCHYTREENODEID activePlatform;
	HierarchyTreeNode::HIERARCHYTREENODEID activeScreen;
};
    
}

#endif /* defined(__UIEditor__BaseCommand__) */
