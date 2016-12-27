#include "ControlPropertiesSection.h"

#include "PropertyVisitor.h"
#include "ValueProperty.h"
#include "IntrospectionProperty.h"

#include "UI/UIControl.h"

using namespace DAVA;

ControlPropertiesSection::ControlPropertiesSection(DAVA::UIControl* aControl, const DAVA::InspInfo* typeInfo, const ControlPropertiesSection* sourceSection, eCloneType cloneType)
    : SectionProperty(typeInfo->Name().c_str())
    , control(SafeRetain(aControl))
{
    for (int i = 0; i < typeInfo->MembersCount(); i++)
    {
        const InspMember* member = typeInfo->Member(i);
        if ((member->Flags() & I_EDIT) != 0)
        {
            IntrospectionProperty* sourceProperty = nullptr == sourceSection ? nullptr : sourceSection->FindProperty(member);
            IntrospectionProperty* prop = IntrospectionProperty::Create(control, member, sourceProperty, cloneType);
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
