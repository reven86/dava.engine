#include "PrototypeNameProperty.h"

#include "PropertyVisitor.h"
#include "../PackageHierarchy/ControlNode.h"
#include "../PackageHierarchy/PackageNode.h"

using namespace DAVA;

PrototypeNameProperty::PrototypeNameProperty(ControlNode *aNode, const PrototypeNameProperty *sourceProperty, eCloneType cloneType)
    : ValueProperty("Prototype")
    , node(aNode) // weak
{
}

PrototypeNameProperty::~PrototypeNameProperty()
{
    node = nullptr; // weak
}

void PrototypeNameProperty::Accept(PropertyVisitor *visitor)
{
    visitor->VisitPrototypeNameProperty(this);
}

AbstractProperty::ePropertyType PrototypeNameProperty::GetType() const
{
    return TYPE_VARIANT;
}

DAVA::VariantType PrototypeNameProperty::GetValue() const
{
    return VariantType(GetPrototypeName());
}

bool PrototypeNameProperty::IsReadOnly() const
{
    return true;
}

String PrototypeNameProperty::GetPrototypeName() const
{
    if (node->GetPrototype())
    {
        if (node->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE_CHILD)
            return node->GetPathToPrototypeChild(true);
        else
            return node->GetPrototype()->GetQualifiedName();
    }
    
    return String("");
}

ControlNode *PrototypeNameProperty::GetControl() const
{
    return node;
}

void PrototypeNameProperty::ApplyValue(const DAVA::VariantType &value)
{
    // do nothing
}
