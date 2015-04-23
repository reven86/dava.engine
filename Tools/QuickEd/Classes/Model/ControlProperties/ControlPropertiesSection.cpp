#include "ControlPropertiesSection.h"

#include "UI/UIControl.h"
#include "ValueProperty.h"
#include "LocalizedTextValueProperty.h"
#include "StringProperty.h"
#include "Base/FunctionTraits.h"

using namespace DAVA;

ControlPropertiesSection::ControlPropertiesSection(DAVA::UIControl *aControl, const DAVA::InspInfo *typeInfo, const ControlPropertiesSection *sourceSection, eCloneType cloneType)
    : control(SafeRetain(aControl))
    , classProperty(nullptr)
    , customClassProperty(nullptr)
    , prototypeProperty(nullptr)
    , nameProperty(nullptr)
{
    name = typeInfo->Name();

    ValueProperty *sourceClassProperty = sourceSection == nullptr ? nullptr : sourceSection->GetClassProperty();
    classProperty = new StringProperty("Class", control, DAVA::MakeFunction(&DAVA::UIControl::GetControlClassName), NULL, dynamic_cast<StringProperty*>(sourceClassProperty), cloneType);
    AddProperty(classProperty);

    ValueProperty *sourceCustomClassProperty = sourceSection == nullptr ? nullptr : sourceSection->GetCustomClassProperty();
    customClassProperty = new StringProperty("Custom class", control, DAVA::MakeFunction(&DAVA::UIControl::GetCustomControlClassName), DAVA::MakeFunction(&DAVA::UIControl::SetCustomControlClassName), dynamic_cast<StringProperty*>(sourceCustomClassProperty), cloneType);
    AddProperty(customClassProperty);

    ValueProperty *sourcePrototypeProperty = sourceSection == nullptr ? nullptr : sourceSection->GetPrototypeProperty();
    prototypeProperty = new StringProperty("Prototype", control, DAVA::MakeFunction(&DAVA::UIControl::GetCustomControlClassName), NULL, dynamic_cast<StringProperty*>(sourcePrototypeProperty), cloneType);
    AddProperty(prototypeProperty);

    ValueProperty *sourceNameProperty = sourceSection == nullptr ? nullptr : sourceSection->GetNameProperty();
    nameProperty = new StringProperty("Name", control, DAVA::MakeFunction(&DAVA::UIControl::GetName), DAVA::MakeFunction(&DAVA::UIControl::SetName), dynamic_cast<StringProperty*>(sourceNameProperty), cloneType);
    AddProperty(nameProperty);
    
    for (int i = 0; i < typeInfo->MembersCount(); i++)
    {
        const InspMember *member = typeInfo->Member(i);
        if ((member->Flags() & I_EDIT) != 0)
        {
            String memberName = member->Name();
            
            ValueProperty *sourceProperty = nullptr == sourceSection ? nullptr : sourceSection->FindProperty(member);

            ValueProperty *prop = strcmp(member->Name(), "text") == 0 ? new LocalizedTextValueProperty(control, member, dynamic_cast<LocalizedTextValueProperty*>(sourceProperty), cloneType)
                                                                      : new IntrospectionProperty(control, member, dynamic_cast<IntrospectionProperty *>(sourceProperty), cloneType);

            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

ControlPropertiesSection::~ControlPropertiesSection()
{
    SafeRelease(classProperty);
    SafeRelease(customClassProperty);
    SafeRelease(prototypeProperty);
    SafeRelease(nameProperty);
    SafeRelease(control);
}

DAVA::String ControlPropertiesSection::GetName() const
{
    return name;
}
