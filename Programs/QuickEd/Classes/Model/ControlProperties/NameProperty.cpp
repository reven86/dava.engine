#include "NameProperty.h"

#include "PropertyVisitor.h"
#include "../PackageHierarchy/ControlNode.h"

#include "UI/UIControl.h"

using namespace DAVA;

NameProperty::NameProperty(ControlNode* controlNode_, const NameProperty* sourceProperty, eCloneType cloneType)
    : ValueProperty("Name", Type::Instance<String>())
    , controlNode(controlNode_) // weak ptr
{
    if (sourceProperty)
    {
        controlNode->GetControl()->SetName(sourceProperty->GetValue().Cast<FastName>());

        if (cloneType == CT_INHERIT && controlNode->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE_CHILD)
        {
            AttachPrototypeProperty(sourceProperty);
        }
    }
}

NameProperty::~NameProperty()
{
    controlNode = nullptr; // weak ptr
}

void NameProperty::Refresh(DAVA::int32 refreshFlags)
{
    ValueProperty::Refresh(refreshFlags);

    if ((refreshFlags & REFRESH_DEFAULT_VALUE) != 0 && GetPrototypeProperty())
        ApplyValue(GetDefaultValue());
}

void NameProperty::Accept(PropertyVisitor* visitor)
{
    visitor->VisitNameProperty(this);
}

bool NameProperty::IsReadOnly() const
{
    return controlNode->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE_CHILD || ValueProperty::IsReadOnly();
}

NameProperty::ePropertyType NameProperty::GetType() const
{
    return TYPE_VARIANT;
}

Any NameProperty::GetValue() const
{
    return Any(controlNode->GetName());
}

bool NameProperty::IsOverriddenLocally() const
{
    return controlNode->GetCreationType() != ControlNode::CREATED_FROM_PROTOTYPE_CHILD;
}

ControlNode* NameProperty::GetControlNode() const
{
    return controlNode;
}

void NameProperty::ApplyValue(const DAVA::Any& value)
{
    if (value.CanGet<String>())
    {
        controlNode->GetControl()->SetName(value.Cast<FastName>());
    }
    else
    {
        DVASSERT(false);
    }
}
