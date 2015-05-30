#include "ControlPropertiesSection.h"

#include "PropertyVisitor.h"
#include "ValueProperty.h"
#include "LocalizedTextValueProperty.h"
#include "FontValueProperty.h"

#include "UI/UIControl.h"

using namespace DAVA;

ControlPropertiesSection::ControlPropertiesSection(DAVA::UIControl *aControl, const DAVA::InspInfo *typeInfo, const ControlPropertiesSection *sourceSection, eCloneType cloneType)
    : SectionProperty(typeInfo->Name())
    , control(SafeRetain(aControl))
{
    for (int i = 0; i < typeInfo->MembersCount(); i++)
    {
        const InspMember *member = typeInfo->Member(i);
        if ((member->Flags() & I_EDIT) != 0)
        {
            String memberName = member->Name();
            
            ValueProperty *sourceProperty = nullptr == sourceSection ? nullptr : sourceSection->FindProperty(member);

            ValueProperty *prop = nullptr;
            //TODO: move it to fabric class
            if (strcmp(member->Name(), "text") == 0)
            {
                prop = new LocalizedTextValueProperty(control, member, dynamic_cast<LocalizedTextValueProperty*>(sourceProperty), cloneType);
            }
            else if (strcmp(member->Name(), "font") == 0)
            {
                prop = new FontValueProperty(control, member, dynamic_cast<FontValueProperty*>(sourceProperty), cloneType);
            }
            else
            {
                prop = new IntrospectionProperty(control, member, dynamic_cast<IntrospectionProperty *>(sourceProperty), cloneType);
            }

            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

ControlPropertiesSection::~ControlPropertiesSection()
{
    SafeRelease(control);
}

void ControlPropertiesSection::Accept(PropertyVisitor *visitor)
{
    visitor->VisitControlSection(this);
}
