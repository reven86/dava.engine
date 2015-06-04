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


#ifndef __UIEditor__HierarchyTreeControlNode__
#define __UIEditor__HierarchyTreeControlNode__

#include "DAVAEngine.h"
#include "HierarchyTreeNode.h"
#include "HierarchyTreeScreenNode.h"
#include "EditorListDelegate.h"

using namespace DAVA;

// "Control" node for the Hierarchy Tree.
class HierarchyTreeControlNode: public HierarchyTreeNode
{
public:
	HierarchyTreeControlNode(HierarchyTreeNode* parent, UIControl* uiObject, const QString& name);
	HierarchyTreeControlNode(HierarchyTreeNode* parent, const HierarchyTreeControlNode* node);
	~HierarchyTreeControlNode();

	HierarchyTreeScreenNode* GetScreenNode() const;
	HierarchyTreeControlNode* GetControlNode() const;
	UIControl* GetUIObject() const {return uiObject;};
	
	virtual void SetParent(HierarchyTreeNode* node, HierarchyTreeNode* insertAfter);
	virtual HierarchyTreeNode* GetParent() {return parent;};
	
	virtual HierarchyTreeControlNode* CreateControlCopy(HierarchyTreeNode* parent) const;
	
	Vector2 GetParentDelta(bool skipControl = false) const;

	// Remove/return Tree Node from the scene.
	virtual void RemoveTreeNodeFromScene();
	virtual void ReturnTreeNodeToScene();
	
	Rect GetRect(bool checkAngle = false) const;

	void SetVisibleFlag(bool value);
	bool GetVisibleFlag() const;

    // Screen scale/position changed.
    virtual void OnScreenScaleChanged();
    virtual void OnScreenPositionChanged();

private:
	void AddControlToParent();
    void UpdateUIObject();
	
private:
	HierarchyTreeNode* parent;

	UIControl* uiObject;
    EditorListDelegate *listDelegate;

	UIControl* parentUIObject;
	bool needReleaseUIObjects;
};

#endif /* defined(__UIEditor__HierarchyTreeControlNode__) */
