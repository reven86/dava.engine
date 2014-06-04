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


#include "AggregatorMetadata.h"
#include "HierarchyTreeController.h"

// Getters/setters.
QString AggregatorMetadata::GetName() const
{
    const HierarchyTreeAggregatorNode* node = GetNode();
    if (node)
    {
        return node->GetName();
    }

    return QString();
}

void AggregatorMetadata::SetName(const QString& name)
{
    HierarchyTreeAggregatorNode* node = GetNode();
    if (node)
    {
        node->SetName(name);
    }
}

float AggregatorMetadata::GetHeight() const
{
    HierarchyTreeAggregatorNode* node = GetNode();
    if (node)
    {
        return node->GetRect().dy;
    }

    return -1.0f;
}

void AggregatorMetadata::SetHeight(float value)
{
    HierarchyTreeAggregatorNode* node = GetNode();
    if (node)
    {
        node->SetRect(Rect(0, 0, node->GetRect().dx, value));
    }
}

float AggregatorMetadata::GetWidth() const
{
    HierarchyTreeAggregatorNode* node = GetNode();
    if (node)
    {
        return node->GetRect().dx;
    }
    
    return -1.0f;
}

void AggregatorMetadata::SetWidth(float value)
{
    HierarchyTreeAggregatorNode* node = GetNode();
    if (node)
    {
        node->SetRect(Rect(0, 0, value, node->GetRect().dy));
    }
}

HierarchyTreeAggregatorNode* AggregatorMetadata::GetNode() const
{
    // Platform Node is one and only.
    return dynamic_cast<HierarchyTreeAggregatorNode*>(GetTreeNode(0));
}

void AggregatorMetadata::ApplyRename(const QString& originalName, const QString& newName)
{
    // Aggregator unused item exactly the same as Screen one.
    HierarchyTreeController::Instance()->AddUnusedItem(new HierarchyTreeController::ScreenUnusedItem(GetNode()->GetPlatform()->GetName(), originalName));

    BaseMetadata::ApplyRename(originalName, newName);
}
