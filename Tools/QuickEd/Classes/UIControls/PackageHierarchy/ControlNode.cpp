#include "ControlNode.h"

using namespace DAVA;

ControlNode::ControlNode(UIControl *control)
    : PackageBaseNode(NULL)
    , control(SafeRetain(control))
    , propertiesRoot(NULL)
    , prototype(NULL)
    , prototypePackage(NULL)
    , creationType(CREATED_FROM_CLASS)
    , readOnly(false)
{
    this->propertiesRoot = new PropertiesRoot(control);
}

ControlNode::ControlNode(ControlNode *prototype, UIPackage *prototypePackage, eCreationType creationType)
    : PackageBaseNode(NULL)
    , control(NULL)
    , propertiesRoot(NULL)
    , prototype(NULL)
    , prototypePackage(NULL)
    , creationType(creationType)
    , readOnly(false)
{
    UIControl *sourceControl = prototype->GetControl();
    control = ObjectFactory::Instance()->New<UIControl>(sourceControl->GetControlClassName());
    control->SetCustomControlClassName(sourceControl->GetCustomControlClassName());
    
    eCreationType childCreationType;
    if (creationType != CREATED_FROM_CLASS)
    {
        this->prototype = SafeRetain(prototype);
        this->prototypePackage = SafeRetain(prototypePackage);
        childCreationType = CREATED_FROM_PROTOTYPE_CHILD;
    }
    else
    {
        this->creationType = CREATED_FROM_CLASS;
        childCreationType = CREATED_FROM_CLASS;
    }

    propertiesRoot = new PropertiesRoot(control, prototype->GetPropertiesRoot());
    
    for (auto it = prototype->nodes.begin(); it != prototype->nodes.end(); ++it)
    {
        ControlNode *childNode = new ControlNode(*it, NULL, childCreationType);
        childNode->SetParent(this);
        nodes.push_back(childNode);
        control->AddControl(childNode->GetControl());
    }
}

ControlNode::~ControlNode()
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
        (*it)->Release();
    nodes.clear();
    
    SafeRelease(control);
    SafeRelease(propertiesRoot);
    SafeRelease(prototype);
    SafeRelease(prototypePackage);
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
        nodes.push_back(SafeRetain(node));
        control->AddControl(node->GetControl());
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

PackageBaseNode *ControlNode::Get(int index) const
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

    if (prototypePackage)
        return prototypePackage->GetName() + "/" + prototype->GetName();

    return prototype->GetName();
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

YamlNode *ControlNode::Serialize(YamlNode *prototypeChildren) const
{
    YamlNode *node = YamlNode::CreateMapNode(false);
    
    if (creationType == CREATED_FROM_PROTOTYPE)
    {
        node->Add("prototype", GetPrototypeName());

        if (!control->GetCustomControlClassName().empty() && prototype->GetControl()->GetCustomControlClassName() != control->GetCustomControlClassName())
            node->Add("customClass", control->GetCustomControlClassName());
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
        node->Add("path", path);
    }
    else
    {
        node->Add("class", control->GetClassName());
        if (!control->GetCustomControlClassName().empty())
            node->Add("customClass", control->GetCustomControlClassName());
    }
    
    propertiesRoot->AddPropertiesToNode(node);
    if (!nodes.empty())
    {
        YamlNode *children = YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION);
        YamlNode *newPrototypeChildren = creationType == CREATED_FROM_PROTOTYPE ? children : prototypeChildren;
        for (auto it = nodes.begin(); it != nodes.end(); ++it)
        {
            ControlNode *child = *it;
            YamlNode *childYamlNode = child->Serialize(newPrototypeChildren);

            switch (child->GetCreationType())
            {
                case CREATED_FROM_CLASS:
                    children->Add(childYamlNode);
                    break;
                case CREATED_FROM_PROTOTYPE:
                    children->Add(childYamlNode);
                    break;
                case CREATED_FROM_PROTOTYPE_CHILD:
                    if (childYamlNode->GetCount() > 1) // for skip empty nodes (one node for path)
                        newPrototypeChildren->Add(childYamlNode);
                    else
                        SafeRelease(childYamlNode);
                    break;
                default:
                    DVASSERT(false);
                    break;
            }
        }
        
        if (children->GetCount() > 0)
            node->Add("children", children);
        else
            SafeRelease(children);
    }
    
    return node;
}
