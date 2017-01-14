#include "ControlPropertiesSection.h"

#include "PropertyVisitor.h"
#include "ValueProperty.h"
#include "IntrospectionProperty.h"

#include "UI/UIControl.h"

using namespace DAVA;

ControlPropertiesSection::ControlPropertiesSection(DAVA::UIControl* aControl, const DAVA::ReflectedType* reflectedType, const ControlPropertiesSection* sourceSection, eCloneType cloneType)
    : SectionProperty(reflectedType->GetPermanentName())
    , control(SafeRetain(aControl))
{
    for (const std::unique_ptr<ReflectedStructure::Field> &field : reflectedType->GetStrucutre()->fields)
    {
        IntrospectionProperty* sourceProperty = nullptr == sourceSection ? nullptr : sourceSection->FindProperty(field.get());
        IntrospectionProperty* prop = IntrospectionProperty::Create(control, field.get(), sourceProperty, cloneType);
        AddProperty(prop);
        SafeRelease(prop);
    }
}

ControlPropertiesSection::~ControlPropertiesSection()
{
    SafeRelease(control);
}

void ControlPropertiesSection::Accept(PropertyVisitor* visitor)
{
    visitor->VisitControlSection(this);
}
