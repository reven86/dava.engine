#include "CustomClassProperty.h"

#include "../PackageHierarchy/ControlNode.h"
#include "../PackageSerializer.h"

#include "UI/UIControl.h"

using namespace DAVA;

CustomClassProperty::CustomClassProperty(ControlNode *aControl, const CustomClassProperty *sourceProperty, eCloneType cloneType)
    : ValueProperty("Custom Class")
    , control(aControl) // weak
    , prototypeProperty(nullptr)
{
    defaultValue = VariantType(aControl->GetControl()->GetCustomControlClassName());
    
    if (sourceProperty)
    {
        aControl->GetControl()->SetCustomControlClassName(sourceProperty->GetValue().AsString());
        
        if (cloneType == CT_COPY)
        {
            defaultValue = sourceProperty->GetDefaultValue();
            replaced = sourceProperty->IsReplaced();
        }
        else
        {
            prototypeProperty = sourceProperty;
            defaultValue = sourceProperty->GetValue();
        }
    }
}

CustomClassProperty::~CustomClassProperty()
{
    control = nullptr; //weak
}

void CustomClassProperty::Refresh()
{
    if (prototypeProperty)
    {
        SetDefaultValue(prototypeProperty->GetValue());
    }
    ValueProperty::Refresh();
}

AbstractProperty *CustomClassProperty::FindPropertyByPrototype(AbstractProperty *prototype)
{
    return prototypeProperty == prototype ? this : nullptr;
}

void CustomClassProperty::Serialize(PackageSerializer *serializer) const
{
    if (IsReplaced())
    {
        serializer->PutValue("customClass", control->GetControl()->GetCustomControlClassName());
    }
}

bool CustomClassProperty::IsReadOnly() const
{
    return control->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE_CHILD || ValueProperty::IsReadOnly();
}

CustomClassProperty::ePropertyType CustomClassProperty::GetType() const
{
    return TYPE_VARIANT;
}

VariantType CustomClassProperty::GetValue() const
{
    return VariantType(control->GetControl()->GetCustomControlClassName());
}

void CustomClassProperty::ApplyValue(const DAVA::VariantType &value)
{
    control->GetControl()->SetCustomControlClassName(value.AsString());
}
