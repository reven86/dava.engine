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


#include "ControlNode.h"

#include "UI/UIControl.h"
#include "Base/ObjectFactory.h"

#include "PackageNode.h"
#include "PackageVisitor.h"
#include "Model/ControlProperties/RootProperty.h"

using namespace DAVA;

static const Set<String> ControlClassesWithoutChildren = {"UI3DView"};

ControlNode::ControlNode(UIControl *control, bool recursively)
    : ControlsContainerNode(nullptr)
    , control(SafeRetain(control))
    , rootProperty(nullptr)
    , prototype(nullptr)
    , creationType(CREATED_FROM_CLASS)
{
    rootProperty = new RootProperty(this, nullptr, AbstractProperty::CT_COPY);
    
    if (recursively)
    {
        const List<UIControl*> &children = control->GetChildren();
        for (UIControl *child : children)
        {
            ControlNode *childNode(new ControlNode(child, recursively));
            childNode->SetParent(this);
            childNode->SetPackageContext(GetPackageContext());
            nodes.push_back(childNode);
        }
    }
}

ControlNode::ControlNode(ControlNode *node, eCreationType _creationType)
    : ControlsContainerNode(nullptr)
    , control(nullptr)
    , rootProperty(nullptr)
    , prototype(nullptr)
    , creationType(_creationType)
{
    control = ObjectFactory::Instance()->New<UIControl>(node->GetControl()->GetClassName());
    control->SetLocalPropertySet(node->GetControl()->GetLocalPropertySet());
    
    eCreationType childCreationType;
    if (creationType == CREATED_FROM_CLASS)
    {
        prototype = SafeRetain(node->prototype);
        rootProperty = new RootProperty(this, node->GetRootProperty(), RootProperty::CT_COPY);
        childCreationType = CREATED_FROM_CLASS;
    }
    else
    {
        prototype = SafeRetain(node);
        prototype->AddControlToInstances(this);
        rootProperty = new RootProperty(this, node->GetRootProperty(), RootProperty::CT_INHERIT);
        childCreationType = CREATED_FROM_PROTOTYPE_CHILD;
    }
    

    for (ControlNode *sourceChild : node->nodes)
    {
        ScopedPtr<ControlNode> childNode(new ControlNode(sourceChild, childCreationType));
        Add(childNode);
    }
}

ControlNode::~ControlNode()
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
        (*it)->Release();
    nodes.clear();
    
    SafeRelease(control);
    SafeRelease(rootProperty);

    if (prototype)
        prototype->RemoveControlFromInstances(this);
    SafeRelease(prototype);
    
    DVASSERT(instances.empty());
}

ControlNode *ControlNode::CreateFromControl(DAVA::UIControl *control)
{
    return new ControlNode(control, false);
}

ControlNode *ControlNode::CreateFromControlWithChildren(UIControl *control)
{
    return new ControlNode(control, true);
}

ControlNode *ControlNode::CreateFromPrototype(ControlNode *sourceNode)
{
    return new ControlNode(sourceNode, CREATED_FROM_PROTOTYPE);
}

ControlNode *ControlNode::CreateFromPrototypeChild(ControlNode *sourceNode)
{
    return new ControlNode(sourceNode, CREATED_FROM_PROTOTYPE_CHILD);
}

ControlNode *ControlNode::Clone()
{
    return new ControlNode(this, CREATED_FROM_CLASS);
}

void ControlNode::Add(ControlNode *node)
{
    DVASSERT(node->GetParent() == nullptr);
    node->SetParent(this);
    nodes.push_back(SafeRetain(node));
    control->AddControl(node->GetControl());
    node->SetPackageContext(GetPackageContext());
}

void ControlNode::InsertAtIndex(int index, ControlNode *node)
{
    if (index >= static_cast<int>(nodes.size()))
    {
        Add(node);
    }
    else
    {
        DVASSERT(node->GetParent() == nullptr);
        node->SetParent(this);
        
        UIControl *belowThis = nodes[index]->GetControl();
        
        nodes.insert(nodes.begin() + index, SafeRetain(node));
        control->InsertChildBelow(node->GetControl(), belowThis);
        node->SetPackageContext(GetPackageContext());
    }
}

void ControlNode::Remove(ControlNode *node)
{
    auto it = find(nodes.begin(), nodes.end(), node);
    if (it != nodes.end())
    {
        DVASSERT(node->GetParent() == this);
        node->SetParent(nullptr);

        node->GetControl()->RemoveFromParent();
        node->SetPackageContext(nullptr);
        nodes.erase(it);
        SafeRelease(node);
    }
    else
    {
        DVASSERT(false);
    }
}

int ControlNode::GetCount() const
{
    return static_cast<int>(nodes.size());
}

ControlNode *ControlNode::Get(int index) const
{
    return nodes[index];
}

void ControlNode::Accept(PackageVisitor *visitor)
{
    visitor->VisitControl(this);
}

ControlNode *ControlNode::FindByName(const DAVA::String &name) const
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
    {
        if ((*it)->GetName() == name)
            return *it;
    }
    return nullptr;
}

String ControlNode::GetName() const
{
    return control->GetName();
}

UIControl *ControlNode::GetControl() const
{
    return control;
}

UIControlPackageContext *ControlNode::GetPackageContext() const
{
    return control->GetPackageContext();
}

void ControlNode::SetPackageContext(UIControlPackageContext *context)
{
    control->SetPackageContext(context);
    for (ControlNode *child : nodes)
        child->SetPackageContext(context);
}

ControlNode *ControlNode::GetPrototype() const
{
    return prototype;
}

const Vector<ControlNode*> &ControlNode::GetInstances() const
{
    return instances;
}

bool ControlNode::IsDependsOnPackage(PackageNode *package) const
{
    if (prototype && prototype->GetPackage() == package)
        return true;
    
    for (ControlNode *child : nodes)
    {
        if (child->IsDependsOnPackage(package))
            return true;
    }
    
    return false;
}

bool ControlNode::IsEditingSupported() const
{
    return !IsReadOnly();
}

bool ControlNode::IsInsertingControlsSupported() const
{
    return !IsReadOnly() && !ControlClassesWithoutChildren.count(control->GetClassName());
}

bool ControlNode::CanInsertControl(ControlNode *node, DAVA::int32 pos) const
{
    if (IsReadOnly() || ControlClassesWithoutChildren.count(control->GetClassName()))
        return false;
    
    if (pos < static_cast<int32>(nodes.size()) && nodes[pos]->GetCreationType() == CREATED_FROM_PROTOTYPE_CHILD)
        return false;
    
    if (node && node->IsInstancedFrom(this))
        return false;
    
    return true;
}

bool ControlNode::CanRemove() const
{
    return !IsReadOnly() && creationType != CREATED_FROM_PROTOTYPE_CHILD;
}

bool ControlNode::CanCopy() const
{
    return creationType != CREATED_FROM_PROTOTYPE_CHILD;
}

void ControlNode::RefreshProperties()
{
    rootProperty->Refresh(AbstractProperty::REFRESH_FONT | AbstractProperty::REFRESH_LOCALIZATION);
    for (ControlNode *node : nodes)
        node->RefreshProperties();
}

void ControlNode::MarkAsRemoved()
{
    if (prototype)
        prototype->RemoveControlFromInstances(this);
    for (ControlNode *node : nodes)
        node->MarkAsRemoved();
}

void ControlNode::MarkAsAlive()
{
    if (prototype)
        prototype->AddControlToInstances(this);
    for (ControlNode *node : nodes)
        node->MarkAsAlive();
}

String ControlNode::GetPathToPrototypeChild() const
{
    if (creationType == CREATED_FROM_PROTOTYPE_CHILD)
    {
        String path = GetName();
        PackageBaseNode *p = GetParent();
        while (p != nullptr && p->GetControl() != nullptr && static_cast<ControlNode*>(p)->GetCreationType() != CREATED_FROM_PROTOTYPE)
        {
            path = p->GetName() + "/" + path;
            p = p->GetParent();
        }
        
        return path;
    }
    return "";
}

bool ControlNode::IsInstancedFrom(const ControlNode *prototype) const
{
    const ControlNode *test = this;
    
    while (test)
    {
        if (test->GetPrototype() != nullptr)
        {
            if (test->GetPrototype() == prototype)
                return true;
            test = test->GetPrototype();
        }
        else
        {
            test = nullptr;
        }
    }
    
    for (const ControlNode *child : nodes)
    {
        if (child->IsInstancedFrom(prototype))
            return true;
    }
    
    return false;
}

void ControlNode::AddControlToInstances(ControlNode *control)
{
    auto it = std::find(instances.begin(), instances.end(), control);
    if (it == instances.end())
        instances.push_back(control);
}

void ControlNode::RemoveControlFromInstances(ControlNode *control)
{
    auto it = std::find(instances.begin(), instances.end(), control);
    if (it != instances.end())
        instances.erase(it);
}
