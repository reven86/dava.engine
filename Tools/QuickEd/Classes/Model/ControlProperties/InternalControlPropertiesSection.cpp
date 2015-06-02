#include "InternalControlPropertiesSection.h"

#include "ValueProperty.h"
#include "UI/UIControl.h"
#include "../PackageSerializer.h"
#include "LocalizedTextValueProperty.h"
#include "FontValueProperty.h"

using namespace DAVA;

InternalControlPropertiesSection::InternalControlPropertiesSection(DAVA::UIControl *control, int num, const InternalControlPropertiesSection *sourceSection, eCopyType copyType) : control(nullptr), internalControl(nullptr), internalControlNum(num)
{
    this->control = SafeRetain(control);
    
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
                prop = new LocalizedTextValueProperty(internalControl, member, sourceProperty, copyType);
            }
            else if (strcmp(member->Name(), "font") == 0)
            {
                prop = new FontValueProperty(internalControl, member, sourceProperty, copyType);
            }
            else
            {
                prop = new ValueProperty(internalControl, member, sourceProperty, copyType);
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
                prop = new LocalizedTextValueProperty(internalControl, member, nullptr, COPY_VALUES);
            }
            else if (strcmp(member->Name(), "font") == 0)
            {
                prop = new FontValueProperty(internalControl, member, nullptr, COPY_VALUES);
            }
            else
            {
                prop = new ValueProperty(internalControl, member, nullptr, COPY_VALUES);
            }
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

DAVA::String InternalControlPropertiesSection::GetName() const
{
    return control->GetInternalControlName(internalControlNum) + control->GetInternalControlDescriptions();
}

bool InternalControlPropertiesSection::HasChanges() const
{
    return internalControl && PropertiesSection::HasChanges();
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

