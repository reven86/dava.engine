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


#include "HierarchyTreeAggregatorControlNode.h"
#include "HierarchyTreeAggregatorNode.h"

using namespace DAVA;

HierarchyTreeAggregatorControlNode::HierarchyTreeAggregatorControlNode(HierarchyTreeAggregatorNode* parentAggregator,
																	   HierarchyTreeNode* parent,
																	   UIControl* uiObject,
																	   const QString& name):
	HierarchyTreeControlNode(parent, uiObject, name),
	parentAggregatorSave(NULL)
{
	this->parentAggregator = parentAggregator;
}

HierarchyTreeAggregatorControlNode::HierarchyTreeAggregatorControlNode(HierarchyTreeNode* parent, const HierarchyTreeAggregatorControlNode* node) :
	HierarchyTreeControlNode(parent, node),
	parentAggregatorSave(NULL)
{
	parentAggregator = node->parentAggregator;
	
	if (parent && parentAggregator)
		parentAggregator->AddChild(this);
}

HierarchyTreeAggregatorControlNode::~HierarchyTreeAggregatorControlNode()
{
	if (parentAggregator)
		parentAggregator->RemoveChild(this);
}

void HierarchyTreeAggregatorControlNode::SetAggregatorNode(HierarchyTreeAggregatorNode* parentAggregator)
{
	if (this->parentAggregator)
		this->parentAggregator->RemoveChild(this);
	
	this->parentAggregator = parentAggregator;
	if (this->parentAggregator)
		this->parentAggregator->AddChild(this);
}

void HierarchyTreeAggregatorControlNode::RemoveTreeNodeFromScene()
{
	parentAggregatorSave = parentAggregator;
	SetAggregatorNode(NULL);
	HierarchyTreeControlNode::RemoveTreeNodeFromScene();
}

void HierarchyTreeAggregatorControlNode::ReturnTreeNodeToScene()
{
	SetAggregatorNode(parentAggregatorSave);
	HierarchyTreeControlNode::ReturnTreeNodeToScene();
}

FilePath HierarchyTreeAggregatorControlNode::GetAggregatorPath() const
{
	UIAggregatorControl* aggregatorControl = dynamic_cast<UIAggregatorControl*>(GetUIObject());
	if (aggregatorControl)
	{
		return aggregatorControl->GetAggregatorPath();
	}

	return FilePath();
}

HierarchyTreeControlNode* HierarchyTreeAggregatorControlNode::CreateControlCopy(HierarchyTreeNode* parent) const
{
	return new HierarchyTreeAggregatorControlNode(parent, this);
}
