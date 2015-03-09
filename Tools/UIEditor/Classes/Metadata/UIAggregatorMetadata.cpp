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



#include "UIAggregatorMetadata.h"
#include "HierarchyTreeController.h"
#include "HierarchyTreeAggregatorControlNode.h"

using namespace DAVA;

void UIAggregatorMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        UIControl* control = this->treeNodeParams[i].GetUIControl();
		
        control->SetName(controlName);
        control->SetPosition(position);
        control->SetInputEnabled(GetInitialInputEnabled());
        
        control->GetBackground()->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    }
}

QString UIAggregatorMetadata::GetUIControlClassName() const
{
	const METADATAPARAMSVECT& params = GetParams();
	if (params.size())
	{
		const BaseMetadataParams& param = params[0];
		HierarchyTreeNode::HIERARCHYTREENODEID id = param.GetTreeNodeID();
		HierarchyTreeAggregatorControlNode* node = dynamic_cast<HierarchyTreeAggregatorControlNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(id));
		if (node)
		{
			const HierarchyTreeAggregatorNode* aggregatorNode = node->GetAggregatorNode();
			if (aggregatorNode)
				return aggregatorNode->GetName();
		}
	}
	
	return UIControlMetadata::GetUIControlClassName();
}
