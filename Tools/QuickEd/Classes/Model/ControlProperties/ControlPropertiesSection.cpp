#include "ControlPropertiesSection.h"

#include "UI/UIControl.h"
#include "ValueProperty.h"
#include "LocalizedTextValueProperty.h"

using namespace DAVA;

ControlPropertiesSection::ControlPropertiesSection(DAVA::UIControl *control, const DAVA::InspInfo *typeInfo, const ControlPropertiesSection *sourceSection, eCloneType copyType) : control(SafeRetain(control))
{
    name = typeInfo->Name();
    
    for (int i = 0; i < typeInfo->MembersCount(); i++)
    {
        const InspMember *member = typeInfo->Member(i);
        if ((member->Flags() & I_EDIT) != 0)
        {
            String memberName = member->Name();
            
            ValueProperty *sourceProperty = nullptr == sourceSection ? nullptr : sourceSection->FindProperty(member);

            ValueProperty *prop = strcmp(member->Name(), "text") == 0 ? new LocalizedTextValueProperty(control, member, dynamic_cast<LocalizedTextValueProperty*>(sourceProperty), copyType)
                                                                      : new IntrospectionProperty(control, member, dynamic_cast<IntrospectionProperty *>(sourceProperty), copyType);

            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

ControlPropertiesSection::~ControlPropertiesSection()
{
    SafeRelease(control);
}

DAVA::String ControlPropertiesSection::GetName() const
{
    return name;
}
