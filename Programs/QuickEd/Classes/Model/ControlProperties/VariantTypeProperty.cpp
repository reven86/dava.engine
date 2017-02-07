#include "VariantTypeProperty.h"

#include "Model/ControlProperties/PropertyVisitor.h"
#include "Model/ControlProperties/IntrospectionProperty.h"
#include "Model/ControlProperties/SubValueProperty.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "UI/Styles/UIStyleSheet.h"

using namespace DAVA;

VariantTypeProperty::VariantTypeProperty(const String& name, Any& vt)
    : ValueProperty(name, vt.GetType())
    , value(vt)
{
    SetOverridden(true);
    
    GenerateBuiltInSubProperties();
}

VariantTypeProperty::~VariantTypeProperty()
{
}

void VariantTypeProperty::Accept(PropertyVisitor* visitor)
{
    // do nothing
}

bool VariantTypeProperty::IsReadOnly() const
{
    return GetParent() == nullptr ? true : GetParent()->IsReadOnly();
}

VariantTypeProperty::ePropertyType VariantTypeProperty::GetType() const
{
    return TYPE_VARIANT;
}

Any VariantTypeProperty::GetValue() const
{
    return value;
}

void VariantTypeProperty::ApplyValue(const DAVA::Any& newValue)
{
    value = newValue;
}
