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


#ifndef __UIEditor__PasteCommand__
#define __UIEditor__PasteCommand__

#include "BaseCommand.h"
#include "CopyPasteController.h"

namespace DAVA
{
	class PasteCommand: public BaseCommand
	{
	public:
		PasteCommand(HierarchyTreeNode* parentNode, CopyPasteController::CopyType copyType, const HierarchyTreeNode::HIERARCHYTREENODESLIST * items);
		virtual ~PasteCommand();
		
		// Execute command.
		virtual void Execute();
		virtual void Rollback();
		virtual bool IsUndoRedoSupported() {return true;};
		
	private:
        int PasteControls(HierarchyTreeNode::HIERARCHYTREENODESLIST*, HierarchyTreeNode *parent);
		int PasteScreens(HierarchyTreeNode::HIERARCHYTREENODESLIST*, HierarchyTreePlatformNode* parent);
		int PastePlatforms(HierarchyTreeNode::HIERARCHYTREENODESLIST*, HierarchyTreeRootNode* parent);
		
		bool IsParentContainsCopyItemName(HierarchyTreeNode* parentNode, HierarchyTreeNode* copyNode);
		
		void UpdateControlName(const HierarchyTreeNode* parent, HierarchyTreeNode* node, bool needCreateNewName) const;
		
		// Undo/Redo-related functionality.
		void ReturnPastedControlsToScene();

		virtual void IncrementUnsavedChanges();
		virtual void DecrementUnsavedChanges();
	private:
		HierarchyTreeNode* parentNode;
		CopyPasteController::CopyType copyType;
		
		// Items to be pasted.
		const HierarchyTreeNode::HIERARCHYTREENODESLIST* items;
		
		// Items were pasted (coy of the items to be pasted).
		HierarchyTreeNode::HIERARCHYTREENODESLIST* newItems;
	};
}

#endif /* defined(__UIEditor__PasteCommand__) */
