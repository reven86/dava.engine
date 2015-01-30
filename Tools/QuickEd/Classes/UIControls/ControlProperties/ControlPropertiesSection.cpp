#include "ControlPropertiesSection.h"

#include "UI/UIControl.h"
#include "ValueProperty.h"
#include "LocalizedTextValueProperty.h"

using namespace DAVA;

ControlPropertiesSection::ControlPropertiesSection(DAVA::UIControl *control, const DAVA::InspInfo *typeInfo, const ControlPropertiesSection *sourceSection, eCopyType copyType) : control(SafeRetain(control))
{
    name = typeInfo->Name();
    
    for (int i = 0; i < typeInfo->MembersCount(); i++)
    {
        const InspMember *member = typeInfo->Member(i);
        if ((member->Flags() & I_EDIT) != 0)
        {
            String memberName = member->Name();
            
            ValueProperty *sourceProperty = sourceSection == NULL ? NULL : sourceSection->FindProperty(member);

            ValueProperty *prop;
            if (String(member->Name()) == "text")
                prop = new LocalizedTextValueProperty(control, member, sourceProperty, copyType);
            else
                prop = new ValueProperty(control, member, sourceProperty, copyType);

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
