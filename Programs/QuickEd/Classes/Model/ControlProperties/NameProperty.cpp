#include "NameProperty.h"

#include "PropertyVisitor.h"
#include "../PackageHierarchy/ControlNode.h"

#include "UI/UIControl.h"

using namespace DAVA;

NameProperty::NameProperty(ControlNode* anControl, const NameProperty* sourceProperty, eCloneType cloneType)
    : ValueProperty("Name", Type::Instance<String>())
    , control(anControl) // weak ptr
{
    if (sourceProperty)
    {
        control->GetControl()->SetName(FastName(sourceProperty->GetValue().Get<String>()));

        if (cloneType == CT_INHERIT && control->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE_CHILD)
        {
            AttachPrototypeProperty(sourceProperty);
        }
    }
}

NameProperty::~NameProperty()
{
    control = nullptr; // weak ptr
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
    return control->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE_CHILD || ValueProperty::IsReadOnly();
}

NameProperty::ePropertyType NameProperty::GetType() const
{
    return TYPE_VARIANT;
}

DAVA::uint32 NameProperty::GetFlags() const
{
    return EF_AFFECTS_STYLES;
}

Any NameProperty::GetValue() const
{
    return Any(control->GetName());
}

bool NameProperty::IsOverriddenLocally() const
{
    return control->GetCreationType() != ControlNode::CREATED_FROM_PROTOTYPE_CHILD;
}

ControlNode* NameProperty::GetControlNode() const
{
    return control;
}

void NameProperty::ApplyValue(const DAVA::Any& value)
{
    if (value.CanGet<String>())
    {
        control->GetControl()->SetName(FastName(value.Get<String>()));
    }
    else
    {
        DVASSERT(false);
    }
}
