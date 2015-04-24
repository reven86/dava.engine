#include "ControlNode.h"

#include "UI/UIControl.h"
#include "Base/ObjectFactory.h"

#include "PackageNode.h"
#include "../PackageSerializer.h"

#include "ControlPrototype.h"

using namespace DAVA;

ControlNode::ControlNode(UIControl *control)
    : ControlsContainerNode(nullptr)
    , control(SafeRetain(control))
    , rootProperty(nullptr)
    , prototype(nullptr)
    , creationType(CREATED_FROM_CLASS)
    , readOnly(false)
{
    rootProperty = new RootProperty(this, nullptr, AbstractProperty::CT_COPY);
}

ControlNode::ControlNode(ControlNode *node)
    : ControlsContainerNode(nullptr)
    , control(nullptr)
    , rootProperty(nullptr)
    , prototype(SafeRetain(node->prototype))
    , creationType(node->creationType)
    , readOnly(false)
{
    control = ObjectFactory::Instance()->New<UIControl>(node->control->GetControlClassName());
    control->SetCustomControlClassName(node->control->GetCustomControlClassName());
    
    rootProperty = new RootProperty(this, node->rootProperty, RootProperty::CT_COPY);
    
    for (ControlNode *sourceChild : nodes)
    {
        RefPtr<ControlNode> childNode(sourceChild->Clone());
        Add(childNode.Get());
    }
}

ControlNode::ControlNode(ControlPrototype *_prototype, eCreationType _creationType)
    : ControlsContainerNode(nullptr)
    , control(nullptr)
    , rootProperty(nullptr)
    , prototype(SafeRetain(_prototype))
    , creationType(_creationType)
    , readOnly(false)
{
    control = ObjectFactory::Instance()->New<UIControl>(prototype->GetControlNode()->GetControl()->GetControlClassName());
    control->SetCustomControlClassName(prototype->GetControlNode()->GetControl()->GetCustomControlClassName());

    rootProperty = new RootProperty(this, prototype->GetControlNode()->GetRootProperty(), RootProperty::CT_INHERIT);
    
    prototype->GetControlNode()->AddControlToInstances(this);

    for (ControlNode *sourceChild : prototype->GetControlNode()->nodes)
    {
        ScopedPtr<ControlPrototype> childPrototype(new ControlPrototype(sourceChild, prototype->GetPackageRef()));
        ScopedPtr<ControlNode> childNode(new ControlNode(childPrototype, CREATED_FROM_PROTOTYPE_CHILD));
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
        prototype->GetControlNode()->RemoveControlFromInstances(this);
    SafeRelease(prototype);
    
    DVASSERT(instances.empty());
}

ControlNode *ControlNode::CreateFromControl(DAVA::UIControl *control)
{
    return new ControlNode(control);
}

ControlNode *ControlNode::CreateFromPrototype(ControlNode *sourceNode, PackageRef *nodePackage)
{
    ScopedPtr<ControlPrototype> prototype(new ControlPrototype(sourceNode, nodePackage));
    return new ControlNode(prototype, CREATED_FROM_PROTOTYPE);
}

ControlNode *ControlNode::CreateFromPrototypeChild(ControlNode *sourceNode, PackageRef *nodePackage)
{
    ScopedPtr<ControlPrototype> prototype(new ControlPrototype(sourceNode, nodePackage));
    return new ControlNode(prototype, CREATED_FROM_PROTOTYPE_CHILD);
}

ControlNode *ControlNode::Clone()
{
    return new ControlNode(this);
}

void ControlNode::RefreshPropertyInInstances(AbstractProperty *property)
{
    for (ControlNode *instance : instances)
    {
        AbstractProperty *instanceProperty = instance->rootProperty->FindPropertyByPrototype(property);
        DVASSERT(instanceProperty);
        if (instanceProperty)
        {
            instance->rootProperty->RefreshProperty(instanceProperty);
            instance->RefreshPropertyInInstances(instanceProperty);
        }
    }
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

ControlPrototype *ControlNode::GetPrototype() const
{
    return prototype;
}

const Vector<ControlNode*> &ControlNode::GetInstances() const
{
    return instances;
}

int ControlNode::GetFlags() const
{
    int flag = 0;
    switch (creationType) {
        case CREATED_FROM_CLASS:
            flag |= FLAG_CONTROL_CREATED_FROM_CLASS;
            break;
        case CREATED_FROM_PROTOTYPE:
            flag |= FLAG_CONTROL_CREATED_FROM_PROTOTYPE;
            break;
        case CREATED_FROM_PROTOTYPE_CHILD:
            flag |= FLAG_CONTROL_CREATED_FROM_PROTOTYPE_CHILD;
            break;
            
        default:
            DVASSERT(false);
            break;
    }
    return readOnly ? (FLAG_READ_ONLY | flag) : flag ;
}

void ControlNode::SetReadOnly()
{
    readOnly = true;
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
        (*it)->SetReadOnly();
}

bool ControlNode::IsEditingSupported() const
{
    return !readOnly;
}

bool ControlNode::IsInsertingSupported() const
{
    return !readOnly;
}

bool ControlNode::CanInsertControl(ControlNode *node, DAVA::int32 pos) const
{
    if (readOnly)
        return false;
    
    if (pos < nodes.size() && nodes[pos]->GetCreationType() == CREATED_FROM_PROTOTYPE_CHILD)
        return false;
    
    if (node && node->IsInstancedFrom(this))
        return false;
    
    return true;
}

bool ControlNode::CanRemove() const
{
    return !readOnly && creationType != CREATED_FROM_PROTOTYPE_CHILD;
}

bool ControlNode::CanCopy() const
{
    return creationType != CREATED_FROM_PROTOTYPE_CHILD;
}

void ControlNode::MarkAsRemoved()
{
    if (prototype)
        prototype->GetControlNode()->RemoveControlFromInstances(this);
}

void ControlNode::MarkAsAlive()
{
    if (prototype)
        prototype->GetControlNode()->AddControlToInstances(this);
}

void ControlNode::Serialize(PackageSerializer *serializer, PackageRef *currentPackage) const
{
    serializer->BeginMap();
    
    if (creationType == CREATED_FROM_PROTOTYPE)
    {
        serializer->PutValue("prototype", prototype->GetName(currentPackage != prototype->GetPackageRef()));

        if (!control->GetCustomControlClassName().empty() && prototype->GetControlNode()->GetControl()->GetCustomControlClassName() != control->GetCustomControlClassName())
            serializer->PutValue("customClass", control->GetCustomControlClassName());

        serializer->PutValue("name", control->GetName());
    }
    else if (creationType == CREATED_FROM_CLASS)
    {
        serializer->PutValue("class", control->GetClassName());
        if (!control->GetCustomControlClassName().empty())
            serializer->PutValue("customClass", control->GetCustomControlClassName());

        serializer->PutValue("name", control->GetName());
    }
    else if (creationType == CREATED_FROM_PROTOTYPE_CHILD)
    {
        String path = GetName();
        PackageBaseNode *p = GetParent();
        while (p != nullptr && p->GetControl() != nullptr && static_cast<ControlNode*>(p)->GetCreationType() != CREATED_FROM_PROTOTYPE)
        {
            path = p->GetName() + "/" + path;
            p = p->GetParent();
        }
        serializer->PutValue("path", path);
    }
    else
    {
        DVASSERT(false);
    }

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
                child->Serialize(serializer, currentPackage);

            for (const auto &child : nodes)
            {
                if (child->GetCreationType() != CREATED_FROM_PROTOTYPE_CHILD)
                    child->Serialize(serializer, currentPackage);
            }
            
            serializer->EndArray();
        }
    }
    
    serializer->EndMap();
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

bool ControlNode::IsInstancedFrom(const ControlNode *prototypeControl) const
{
    const ControlNode *test = this;
    
    while (test)
    {
        ControlPrototype *prototype = test->GetPrototype();
        if (prototype != nullptr)
        {
            if (prototype->GetControlNode() == prototypeControl)
                return true;
            test = prototype->GetControlNode();
        }
        else
        {
            test = nullptr;
        }
    }
    
    for (const ControlNode *child : nodes)
    {
        if (child->IsInstancedFrom(prototypeControl))
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
