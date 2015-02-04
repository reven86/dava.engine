#include "ControlNode.h"

#include "UI/UIControl.h"
#include "Base/ObjectFactory.h"

#include "PackageNode.h"
#include "../PackageSerializer.h"

#include "ControlPrototype.h"

using namespace DAVA;

ControlNode::ControlNode(UIControl *control, PropertiesRoot *_propertiesRoot, eCreationType creationType)
    : PackageBaseNode(NULL)
    , control(SafeRetain(control))
    , propertiesRoot(SafeRetain(_propertiesRoot))
    , prototype(NULL)
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
    SafeRelease(prototype);
}

ControlNode *ControlNode::CreateFromControl(DAVA::UIControl *control)
{
    PropertiesRoot *propertiesRoot = new PropertiesRoot(control);
    ControlNode *node = new ControlNode(control, propertiesRoot, CREATED_FROM_CLASS);
    SafeRelease(propertiesRoot);
    return node;
}

ControlNode *ControlNode::CreateFromPrototype(ControlPrototype *prototype)
{
    ControlNode *node = CreateFromPrototypeImpl(prototype->GetControlNode(), true);
    node->prototype = SafeRetain(prototype);
    
    return node;
}

ControlNode *ControlNode::CreateFromPrototypeImpl(ControlNode *prototypeChild, bool root)
{
    RefPtr<UIControl> newControl(ObjectFactory::Instance()->New<UIControl>(prototypeChild->GetControl()->GetControlClassName()));
    newControl->SetCustomControlClassName(prototypeChild->GetControl()->GetCustomControlClassName());
    
    RefPtr<PropertiesRoot> propertiesRoot(new PropertiesRoot(newControl.Get(),
                                                             prototypeChild->GetPropertiesRoot(), PropertiesRoot::COPY_VALUES));
    
    ControlNode *node = new ControlNode(newControl.Get(), propertiesRoot.Get(), root ? CREATED_FROM_PROTOTYPE : CREATED_FROM_PROTOTYPE_CHILD);
    
    for (ControlNode *sourceChild : prototypeChild->nodes)
    {
        RefPtr<ControlNode> childNode(CreateFromPrototypeImpl(sourceChild, false));
        node->Add(childNode.Get());
    }
    
    return node;
    
}

ControlNode *ControlNode::Clone()
{
    RefPtr<UIControl> newControl(ObjectFactory::Instance()->New<UIControl>(control->GetControlClassName()));
    newControl->SetCustomControlClassName(control->GetCustomControlClassName());

    RefPtr<PropertiesRoot> newPropRoot(new PropertiesRoot(newControl.Get(), propertiesRoot, PropertiesRoot::COPY_FULL));

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
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);
    nodes.push_back(SafeRetain(node));
    control->AddControl(node->GetControl());
    node->GetControl()->UpdateLayout();
}

void ControlNode::InsertBelow(ControlNode *node, const ControlNode *belowThis)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);
    auto it = find(nodes.begin(), nodes.end(), belowThis);
    if (it != nodes.end())
    {
        control->InsertChildBelow(node->GetControl(), (*it)->GetControl());
        nodes.insert(it, SafeRetain(node));
    }
    else
    {
        control->AddControl(node->GetControl());
        nodes.push_back(SafeRetain(node));
    }
}

void ControlNode::Remove(ControlNode *node)
{
    auto it = find(nodes.begin(), nodes.end(), node);
    if (it != nodes.end())
    {
        DVASSERT(node->GetParent() == this);
        node->SetParent(NULL);

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
    return NULL;
}

String ControlNode::GetName() const
{
    return control->GetName();
}

UIControl *ControlNode::GetControl() const
{
    return control;
}

String ControlNode::GetPrototypeName() const
{
    if (!prototype)
        return "";

    return prototype->GetName();
}

ControlPrototype *ControlNode::GetPrototype() const
{
    return prototype;
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

void ControlNode::Serialize(PackageSerializer *serializer) const
{
    serializer->BeginMap();
    
    if (creationType == CREATED_FROM_PROTOTYPE)
    {
        serializer->PutValue("prototype", GetPrototypeName());

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
        while (p != NULL && p->GetControl() != NULL && static_cast<ControlNode*>(p)->GetCreationType() != CREATED_FROM_PROTOTYPE)
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
