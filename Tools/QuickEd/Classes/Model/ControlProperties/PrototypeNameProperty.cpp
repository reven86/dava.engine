#include "PrototypeNameProperty.h"

#include "../PackageHierarchy/ControlNode.h"
#include "../PackageHierarchy/PackageNode.h"
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
        String name = "";
        if (node->GetPrototype()->GetPackage()->IsImported())
            name += node->GetPrototype()->GetPackage()->GetName() + "/";
        name += node->GetPrototype()->GetName();
        serializer->PutValue("prototype", name);
    }
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
        {
            String name = "";
            if (node->GetPrototype()->GetPackage()->IsImported())
                name += node->GetPrototype()->GetPackage()->GetName() + "/";
            name += node->GetPrototype()->GetName();
            return name;
        }
    }
    
    return String("");
}

void PrototypeNameProperty::ApplyValue(const DAVA::VariantType &value)
{
    // do nothing
}
