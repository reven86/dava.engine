#include "InternalControlPropertiesSection.h"

#include "PropertyVisitor.h"
#include "ValueProperty.h"
#include "Model/PackageSerializer.h"
#include "LocalizedTextValueProperty.h"
#include "FontValueProperty.h"

#include "UI/UIControl.h"

using namespace DAVA;

InternalControlPropertiesSection::InternalControlPropertiesSection(DAVA::UIControl *aControl, int num, const InternalControlPropertiesSection *sourceSection, eCloneType cloneType)
    : SectionProperty("")
    , control(SafeRetain(aControl))
    , internalControl(nullptr)
    , internalControlNum(num)
{
    name = control->GetInternalControlName(internalControlNum) + control->GetInternalControlDescriptions();
    
    internalControl = SafeRetain(control->GetInternalControl(num));
    if (internalControl == nullptr && sourceSection != nullptr && sourceSection->GetInternalControl() != nullptr)
    {
        internalControl = control->CreateInternalControl(num);
        control->SetInternalControl(num, internalControl);
    }
    
    if (internalControl)
    {
        const InspInfo *insp = internalControl->GetTypeInfo();
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember *member = insp->Member(j);
            
            ValueProperty *sourceProperty = nullptr == sourceSection ? nullptr : sourceSection->FindProperty(member);

            ValueProperty *prop = nullptr;
            //TODO: move it to fabric class
            if (strcmp(member->Name(), "text") == 0)
            {
                prop = new LocalizedTextValueProperty(internalControl, member, dynamic_cast<LocalizedTextValueProperty*>(sourceProperty), cloneType);
            }
            else if (strcmp(member->Name(), "font") == 0)
            {
                prop = new FontValueProperty(internalControl, member, dynamic_cast<FontValueProperty*>(sourceProperty), cloneType);
            }
            else
            {
                prop = new IntrospectionProperty(internalControl, member, dynamic_cast<IntrospectionProperty*>(sourceProperty), cloneType);
            }
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

InternalControlPropertiesSection::~InternalControlPropertiesSection()
{
    SafeRelease(internalControl);
    SafeRelease(control);
}

UIControl *InternalControlPropertiesSection::GetInternalControl() const
{
    return internalControl;
}

void InternalControlPropertiesSection::CreateInternalControl()
{
    if (!internalControl)
    {
        internalControl = control->CreateInternalControl(internalControlNum);
        control->SetInternalControl(internalControlNum, internalControl);
        
        const InspInfo *insp = internalControl->GetTypeInfo();
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember *member = insp->Member(j);

            ValueProperty *prop = nullptr;
            //TODO: move it to fabric class
            if (strcmp(member->Name(), "text") == 0)
            {
                prop = new LocalizedTextValueProperty(internalControl, member, nullptr, CT_COPY);
            }
            else if (strcmp(member->Name(), "font") == 0)
            {
                prop = new FontValueProperty(internalControl, member, nullptr, CT_COPY);
            }
            else
            {
                prop = new IntrospectionProperty(internalControl, member, nullptr, CT_COPY);
            }
            
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

bool InternalControlPropertiesSection::HasChanges() const
{
    return internalControl && SectionProperty::HasChanges();
}

void InternalControlPropertiesSection::Serialize(PackageSerializer *serializer) const
{
    if (HasChanges())
    {
        serializer->BeginMap(GetName());
        
        for (const auto child : children)
            child->Serialize(serializer);
        
        serializer->EndMap();
    }
}

void InternalControlPropertiesSection::Accept(PropertyVisitor *visitor)
{
    visitor->VisitInternalControlSection(this);
}
