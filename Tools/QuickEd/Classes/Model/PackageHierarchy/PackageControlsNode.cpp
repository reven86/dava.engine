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


#include "PackageControlsNode.h"

#include "ControlNode.h"
#include "PackageVisitor.h"

#include "PackageNode.h"
#include "UI/UIPackage.h"
#include "UI/UIControl.h"

using namespace DAVA;

PackageControlsNode::PackageControlsNode(PackageNode *_parent)
    : ControlsContainerNode(_parent)
{
}

PackageControlsNode::~PackageControlsNode()
{
    for (ControlNode *node : nodes)
        node->Release();
    nodes.clear();
}

void PackageControlsNode::Add(ControlNode *node)
{
    DVASSERT(node->GetParent() == nullptr);
    DVASSERT(node->GetPackageContext() == nullptr);
    node->SetParent(this);
    node->SetPackageContext(GetPackage()->GetContext());
    nodes.push_back(SafeRetain(node));
}

void PackageControlsNode::InsertAtIndex(int index, ControlNode *node)
{
    DVASSERT(node->GetParent() == nullptr);
    DVASSERT(node->GetPackageContext() == nullptr);
    node->SetParent(this);
    node->SetPackageContext(GetPackage()->GetContext());
    nodes.insert(nodes.begin() + index, SafeRetain(node));
}

void PackageControlsNode::Remove(ControlNode *node)
{
    auto it = find(nodes.begin(), nodes.end(), node);
    if (it != nodes.end())
    {
        DVASSERT(node->GetParent() == this);
        node->SetParent(nullptr);
        
        DVASSERT(node->GetPackageContext() == GetPackage()->GetContext());
        node->SetPackageContext(nullptr);

        nodes.erase(it);
        SafeRelease(node);
    }
    else
    {
        DVASSERT(false);
    }
}

int PackageControlsNode::GetCount() const
{
    return static_cast<int>(nodes.size());
}

ControlNode *PackageControlsNode::Get(int index) const
{
    return nodes[index];
}

void PackageControlsNode::Accept(PackageVisitor *visitor)
{
    visitor->VisitControls(this);
}

String PackageControlsNode::GetName() const
{
    return "Controls";
}

bool PackageControlsNode::IsEditingSupported() const
{
    return false;
}

bool PackageControlsNode::IsInsertingControlsSupported() const
{
    return !IsReadOnly();
}

bool PackageControlsNode::CanInsertControl(ControlNode *node, DAVA::int32 pos) const
{
    return !IsReadOnly();
}

bool PackageControlsNode::CanRemove() const
{
    return false;
}

bool PackageControlsNode::CanCopy() const
{
    return false;
}

void PackageControlsNode::RefreshControlProperties()
{
    for (ControlNode *node : nodes)
        node->RefreshProperties();
}

ControlNode *PackageControlsNode::FindControlNodeByName(const DAVA::String &name) const
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
    {
        if ((*it)->GetControl()->GetName() == name)
            return *it;
    }
    return NULL;
}
