#include "ControlNode.h"

#include "UI/UIControl.h"
#include "Base/ObjectFactory.h"

#include "PackageNode.h"
#include "PackageVisitor.h"
#include "../PackageSerializer.h"
#include "../ControlProperties/RootProperty.h"

using namespace DAVA;

ControlNode::ControlNode(UIControl *control)
    : ControlsContainerNode(nullptr)
    , control(SafeRetain(control))
    , rootProperty(nullptr)
    , prototype(nullptr)
    , creationType(CREATED_FROM_CLASS)
{
    rootProperty = new RootProperty(this, nullptr, AbstractProperty::CT_COPY);
}

ControlNode::ControlNode(ControlNode *node, eCreationType _creationType)
    : ControlsContainerNode(nullptr)
    , control(nullptr)
    , rootProperty(nullptr)
    , prototype(nullptr)
    , creationType(_creationType)
{
    control = ObjectFactory::Instance()->New<UIControl>(node->GetControl()->GetClassName());
    
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
    return new ControlNode(control);
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
    node->GetControl()->UpdateLayout();
}

void ControlNode::InsertAtIndex(int index, ControlNode *node)
{
    if (index >= nodes.size())
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
        node->GetControl()->UpdateLayout();
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
    return (int) nodes.size();
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

String ControlNode::GetQualifiedName(bool forceQualifiedName) const
{
    const PackageNode *package = GetPackage();
    if (package && (forceQualifiedName || package->IsImported()))
    {
        return package->GetName() + "/" + GetName();
    }
    
    return GetName();
}

UIControl *ControlNode::GetControl() const
{
    return control;
}

ControlNode *ControlNode::GetPrototype() const
{
    return prototype;
}

const Vector<ControlNode*> &ControlNode::GetInstances() const
{
    return instances;
}

bool ControlNode::IsEditingSupported() const
{
    return !IsReadOnly();
}

bool ControlNode::IsInsertingSupported() const
{
    return !IsReadOnly();
}

bool ControlNode::CanInsertControl(ControlNode *node, DAVA::int32 pos) const
{
    if (IsReadOnly())
        return false;
    
    if (pos < nodes.size() && nodes[pos]->GetCreationType() == CREATED_FROM_PROTOTYPE_CHILD)
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
    rootProperty->Refresh();
    for (ControlNode *node : nodes)
        node->RefreshProperties();
}

void ControlNode::MarkAsRemoved()
{
    if (prototype)
        prototype->RemoveControlFromInstances(this);
}

void ControlNode::MarkAsAlive()
{
    if (prototype)
        prototype->AddControlToInstances(this);
}

void ControlNode::Serialize(PackageSerializer *serializer) const
{
    serializer->BeginMap();
    
    rootProperty->Serialize(serializer);
    
    if (!nodes.empty())
    {
        bool shouldProcessChildren = true;
        Vector<ControlNode*> prototypeChildrenWithChanges;

        if (creationType == CREATED_FROM_PROTOTYPE)
        {
            CollectPrototypeChildrenWithChanges(prototypeChildrenWithChanges);
            shouldProcessChildren = !prototypeChildrenWithChanges.empty() || HasNonPrototypeChildren();
        }
        
        if (shouldProcessChildren)
        {
            serializer->BeginArray("children");

            for (const auto &child : prototypeChildrenWithChanges)
                child->Serialize(serializer);

            for (const auto &child : nodes)
            {
                if (child->GetCreationType() != CREATED_FROM_PROTOTYPE_CHILD)
                    child->Serialize(serializer);
            }
            
            serializer->EndArray();
        }
    }
    
    serializer->EndMap();
}

String ControlNode::GetPathToPrototypeChild(bool withRootPrototypeName) const
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
        
        if (withRootPrototypeName && p != nullptr && p->GetControl() != nullptr && static_cast<ControlNode*>(p)->GetCreationType() == CREATED_FROM_PROTOTYPE)
        {
            ControlNode *c = static_cast<ControlNode*>(p);
            if (c->GetPrototype())
            {
                path = c->GetPrototype()->GetQualifiedName() + "/" + path;
            }
        }
        
        return path;
    }
    return "";
}

void ControlNode::CollectPrototypeChildrenWithChanges(Vector<ControlNode*> &out) const
{
    for (auto child : nodes)
    {
        if (child->GetCreationType() == CREATED_FROM_PROTOTYPE_CHILD)
        {
            if (child->HasNonPrototypeChildren() || child->rootProperty->HasChanges())
                out.push_back(child);
            
            child->CollectPrototypeChildrenWithChanges(out);
        }
    }
}

bool ControlNode::HasNonPrototypeChildren() const
{
    for (const auto &child : nodes)
    {
        if (child->GetCreationType() != CREATED_FROM_PROTOTYPE_CHILD)
            return true;
    }
    return false;
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
