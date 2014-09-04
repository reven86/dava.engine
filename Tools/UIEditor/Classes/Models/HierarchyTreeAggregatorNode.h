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




#ifndef __UIEditor__HierarchyTreeAggregatorNode__
#define __UIEditor__HierarchyTreeAggregatorNode__

#include "HierarchyTreeScreenNode.h"
#include "HierarchyTreeControlNode.h"
#include "EditorListDelegate.h"

namespace DAVA
{
	
class HierarchyTreeAggregatorNode: public HierarchyTreeScreenNode
{
public:
	typedef Set<HierarchyTreeControlNode*> CHILDS;

	HierarchyTreeAggregatorNode(HierarchyTreePlatformNode* parent, const QString& name, const Rect& rect);
	HierarchyTreeAggregatorNode(HierarchyTreePlatformNode* parent, const HierarchyTreeAggregatorNode* base, bool needLoad = true);
	~HierarchyTreeAggregatorNode();

	void AddChild(HierarchyTreeControlNode* node);
	void RemoveChild(HierarchyTreeControlNode* node);
	const CHILDS& GetChilds() const;
	
	HierarchyTreeControlNode* CreateChild(HierarchyTreeNode* parentNode, const QString& name);
	void UpdateChilds();
	
	void SetRect(const Rect& rect);
	virtual Rect GetRect() const;
	virtual void SetParent(HierarchyTreeNode* node, HierarchyTreeNode* insertAfter);
	virtual void RemoveSelection();
	
	bool Load(const Rect& rect, const QString& path);
	bool Save(YamlNode* node, const QString& path, bool saveAll);
    
    virtual void ReturnTreeNodeToScene();	
	virtual void SetName(const QString& name);
	
	const FilePath& GetPath();

	void UpdateHierarchyTree();
    
protected:
    virtual Rect GetOwnRect() const;

private:
	void CopyAggregatorControls();
	void ReplaceAggregator(HierarchyTreeControlNode* node);

private:
	Rect rect;
	
	CHILDS childs;
	
	FilePath path;
    
    EditorListDelegate* listDelegate;
};
	
}

#endif /* defined(__UIEditor__HierarchyTreeAggregatorNode__) */
