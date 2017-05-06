#include "ControlPropertiesSection.h"

#include "PropertyVisitor.h"
#include "ValueProperty.h"
#include "IntrospectionProperty.h"

#include "UI/UIControl.h"

using namespace DAVA;

ControlPropertiesSection::ControlPropertiesSection(const DAVA::String& name, DAVA::UIControl* control_, const DAVA::Type* type, const Vector<Reflection::Field>& fields, const ControlPropertiesSection* sourceSection, eCloneType cloneType)
    : SectionProperty(name)
    , control(SafeRetain(control_))
{
    for (const Reflection::Field& field : fields)
    {
        if (field.inheritFrom->GetType() == type)
        {
            String name = field.key.Cast<String>();
            IntrospectionProperty* sourceProperty = nullptr == sourceSection ? nullptr : sourceSection->FindChildPropertyByName(name);
            IntrospectionProperty* prop = IntrospectionProperty::Create(control, -1, name, field.ref, sourceProperty, cloneType);
            AddProperty(prop);
            SafeRelease(prop);
        }
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
