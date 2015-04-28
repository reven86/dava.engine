#include "PrototypeNameProperty.h"

#include "../PackageHierarchy/ControlNode.h"
#include "../PackageHierarchy/ControlPrototype.h"
#include "../PackageSerializer.h"

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

void PrototypeNameProperty::Serialize(PackageSerializer *serializer) const
{
    if (node->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE)
    {
        bool includePackageName = node->GetPackageRef() != node->GetPrototype()->GetPackageRef();
        serializer->PutValue("prototype", node->GetPrototype()->GetName(includePackageName));
    }
}

AbstractProperty::ePropertyType PrototypeNameProperty::GetType() const
{
    return TYPE_VARIANT;
}

DAVA::VariantType PrototypeNameProperty::GetValue() const
{
    if (node->GetPrototype())
    {
        if (node->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE_CHILD)
        {
            return VariantType(node->GetPathToPrototypeChild(true));
        }
        else
        {
            return VariantType(node->GetPrototype()->GetName(true));
        }
    }

    return VariantType(String("-"));
}

bool PrototypeNameProperty::IsReadOnly() const
{
    return true;
}

void PrototypeNameProperty::ApplyValue(const DAVA::VariantType &value)
{
    // do nothing
}
