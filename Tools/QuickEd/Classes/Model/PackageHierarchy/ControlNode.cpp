#include "ControlNode.h"

#include "UI/UIControl.h"
#include "Base/ObjectFactory.h"

#include "PackageNode.h"
#include "../PackageSerializer.h"

#include "ControlPrototype.h"

using namespace DAVA;

ControlNode::ControlNode(UIControl *control, RootProperty *_propertiesRoot, eCreationType creationType)
    : ControlsContainerNode(nullptr)
    , control(SafeRetain(control))
    , propertiesRoot(SafeRetain(_propertiesRoot))
    , prototype(nullptr)
    , creationType(creationType)
    , readOnly(false)
{
}

ControlNode::~ControlNode()
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
        (*it)->Release();
    nodes.clear();
    
    SafeRelease(control);
    SafeRelease(propertiesRoot);

    if (prototype)
        prototype->GetControlNode()->RemoveControlFromInstances(this);
    SafeRelease(prototype);
    
    DVASSERT(instances.empty());
}

ControlNode *ControlNode::CreateFromControl(DAVA::UIControl *control)
{
    RootProperty *propertiesRoot = new RootProperty(control);
    ControlNode *node = new ControlNode(control, propertiesRoot, CREATED_FROM_CLASS);
    SafeRelease(propertiesRoot);
    return node;
}

ControlNode *ControlNode::CreateFromPrototype(ControlNode *sourceNode, PackageRef *nodePackage)
{
    ControlNode *node = CreateFromPrototypeImpl(sourceNode, nodePackage, true);
    return node;
}

ControlNode *ControlNode::CreateFromPrototypeChild(ControlNode *sourceNode, PackageRef *nodePackage)
{
    ControlNode *node = CreateFromPrototypeImpl(sourceNode, nodePackage, false);
    return node;
}

ControlNode *ControlNode::CreateFromPrototypeImpl(ControlNode *sourceNode, PackageRef *nodePackage, bool root)
{
    RefPtr<UIControl> newControl(ObjectFactory::Instance()->New<UIControl>(sourceNode->GetControl()->GetControlClassName()));
    newControl->SetCustomControlClassName(sourceNode->GetControl()->GetCustomControlClassName());
    
    RefPtr<RootProperty> propertiesRoot(new RootProperty(newControl.Get(),
                                                             sourceNode->GetPropertiesRoot(), RootProperty::COPY_VALUES));
    
    ControlNode *node = new ControlNode(newControl.Get(), propertiesRoot.Get(), root ? CREATED_FROM_PROTOTYPE : CREATED_FROM_PROTOTYPE_CHILD);
    node->prototype = new ControlPrototype(sourceNode, nodePackage);
    sourceNode->AddControlToInstances(node);

    for (ControlNode *sourceChild : sourceNode->nodes)
    {
        RefPtr<ControlNode> childNode(CreateFromPrototypeImpl(sourceChild, nodePackage, false));
        node->Add(childNode.Get());
    }
    
    return node;
    
}

ControlNode *ControlNode::Clone()
{
    RefPtr<UIControl> newControl(ObjectFactory::Instance()->New<UIControl>(control->GetControlClassName()));
    newControl->SetCustomControlClassName(control->GetCustomControlClassName());

    RefPtr<RootProperty> newPropRoot(new RootProperty(newControl.Get(), propertiesRoot, RootProperty::COPY_FULL));

    ControlNode *node = new ControlNode(newControl.Get(), newPropRoot.Get(), creationType);
    node->prototype = SafeRetain(prototype);

    for (ControlNode *sourceChild : nodes)
    {
        RefPtr<ControlNode> childNode(sourceChild->Clone());
        node->Add(childNode.Get());
    }

    return node;
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
    propertiesRoot->SetReadOnly();
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

AbstractProperty *ControlNode::GetPropertyByPath(const DAVA::Vector<DAVA::String> &path)
{
    return propertiesRoot->GetPropertyByPath(path);
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
    }
    else if (creationType == CREATED_FROM_CLASS)
    {
        serializer->PutValue("class", control->GetClassName());
        if (!control->GetCustomControlClassName().empty())
            serializer->PutValue("customClass", control->GetCustomControlClassName());
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
    
    propertiesRoot->Serialize(serializer);
    
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
            if (child->HasNonPrototypeChildren() || child->propertiesRoot->HasChanges())
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
