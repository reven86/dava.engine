#include "PropertiesRoot.h"

#include "UI/UIControl.h"

#include "ControlPropertiesSection.h"
#include "ComponentPropertiesSection.h"

#include "BackgroundPropertiesSection.h"
#include "InternalControlPropertiesSection.h"

#include "ValueProperty.h"
#include "LocalizedTextValueProperty.h"

#include "../PackageSerializer.h"

using namespace DAVA;

PropertiesRoot::PropertiesRoot(UIControl *_control)
    : control(SafeRetain(_control))
{
    MakeControlPropertiesSection(control, control->GetTypeInfo(), nullptr, COPY_VALUES);
    MakeBackgroundPropertiesSection(control, nullptr, COPY_VALUES);
    MakeInternalControlPropertiesSection(control, nullptr, COPY_VALUES);
}

PropertiesRoot::PropertiesRoot(UIControl *_control, const PropertiesRoot *sourceProperties, eCopyType copyType)
    : control(SafeRetain(_control))
{
    MakeControlPropertiesSection(control, control->GetTypeInfo(), sourceProperties, copyType);
    MakeBackgroundPropertiesSection(control, sourceProperties, copyType);
    MakeInternalControlPropertiesSection(control, sourceProperties, copyType);
}

PropertiesRoot::~PropertiesRoot()
{
    SafeRelease(control);
    
    for (ControlPropertiesSection *section : controlProperties)
    {
        section->SetParent(nullptr);
        section->Release();
    }
    controlProperties.clear();

    for (BackgroundPropertiesSection *section : backgroundProperties)
    {
        section->SetParent(nullptr);
        section->Release();
    }
    backgroundProperties.clear();

    for (InternalControlPropertiesSection *section : internalControlProperties)
    {
        section->SetParent(nullptr);
        section->Release();
    }
    internalControlProperties.clear();
}

int PropertiesRoot::GetCount() const
{
    return (int) (controlProperties.size() + componentProperties.size() + backgroundProperties.size() + internalControlProperties.size());
}

BaseProperty *PropertiesRoot::GetProperty(int index) const
{
    if (index < (int) controlProperties.size())
        return controlProperties[index];
    index -= controlProperties.size();
    
    if (index < (int) componentProperties.size())
        return componentProperties[index];
    index -= componentProperties.size();
    
    if (index < (int) backgroundProperties.size())
        return backgroundProperties[index];
    index -= backgroundProperties.size();
    
    return internalControlProperties[index];
}

ControlPropertiesSection *PropertiesRoot::GetControlPropertiesSection(const DAVA::String &name) const
{
    for (auto it = controlProperties.begin(); it != controlProperties.end(); ++it)
    {
        if ((*it)->GetName() == name)
            return *it;
    }
    return nullptr;
}

ComponentPropertiesSection *PropertiesRoot::AddComponentPropertiesSection(DAVA::uint32 componentType)
{
    ComponentPropertiesSection *section = new ComponentPropertiesSection(control, (UIComponent::eType) componentType, nullptr, COPY_VALUES);
    componentProperties.push_back(section);
    return section;
}

BackgroundPropertiesSection *PropertiesRoot::GetBackgroundPropertiesSection(int num) const
{
    if (0 <= num && num < (int) backgroundProperties.size())
        return backgroundProperties[num];
    return nullptr;
}

InternalControlPropertiesSection *PropertiesRoot::GetInternalControlPropertiesSection(int num) const
{
    if (0 <= num && num < (int) internalControlProperties.size())
        return internalControlProperties[num];
    return nullptr;
}

void PropertiesRoot::Serialize(PackageSerializer *serializer) const
{
    for (const auto section : controlProperties)
        section->Serialize(serializer);

    bool hasChanges = componentProperties.size() > 0;
    
    if (!hasChanges)
    {
        for (const auto section : backgroundProperties)
        {
            if (section->HasChanges())
            {
                hasChanges = true;
                break;
            }
        }
    }
    
    if (!hasChanges)
    {
        for (const auto section : internalControlProperties)
        {
            if (section->HasChanges())
            {
                hasChanges = true;
                break;
            }
        }
    }


    if (hasChanges)
    {
        serializer->BeginMap("components");

        for (const auto section : componentProperties)
            section->Serialize(serializer);
        
        for (const auto section : backgroundProperties)
            section->Serialize(serializer);

        for (const auto section : internalControlProperties)
            section->Serialize(serializer);
        
        serializer->EndArray();
    }
}

String PropertiesRoot::GetName() const {
    return "ROOT";
}

BaseProperty::ePropertyType PropertiesRoot::GetType() const {
    return TYPE_HEADER;
}

void PropertiesRoot::MakeControlPropertiesSection(DAVA::UIControl *control, const DAVA::InspInfo *typeInfo, const PropertiesRoot *sourceProperties, eCopyType copyType)
{
    const InspInfo *baseInfo = typeInfo->BaseInfo();
    if (baseInfo)
        MakeControlPropertiesSection(control, baseInfo, sourceProperties, copyType);
    
    bool hasProperties = false;
    for (int i = 0; i < typeInfo->MembersCount(); i++)
    {
        const InspMember *member = typeInfo->Member(i);
        if ((member->Flags() & I_EDIT) != 0)
        {
            hasProperties = true;
            break;
        }
    }
    if (hasProperties)
    {
        ControlPropertiesSection *sourceSection = sourceProperties == nullptr ? nullptr : sourceProperties->GetControlPropertiesSection(typeInfo->Name());
        ControlPropertiesSection *section = new ControlPropertiesSection(control, typeInfo, sourceSection, copyType);
        section->SetParent(this);
        controlProperties.push_back(section);
    }
}

void PropertiesRoot::MakeBackgroundPropertiesSection(DAVA::UIControl *control, const PropertiesRoot *sourceProperties, eCopyType copyType)
{
    for (int i = 0; i < control->GetBackgroundComponentsCount(); i++)
    {
        BackgroundPropertiesSection *sourceSection = sourceProperties == nullptr ? nullptr : sourceProperties->GetBackgroundPropertiesSection(i);
        BackgroundPropertiesSection *section = new BackgroundPropertiesSection(control, i, sourceSection, copyType);
        section->SetParent(this);
        backgroundProperties.push_back(section);
    }
}

void PropertiesRoot::MakeInternalControlPropertiesSection(DAVA::UIControl *control, const PropertiesRoot *sourceProperties, eCopyType copyType)
{
    for (int i = 0; i < control->GetInternalControlsCount(); i++)
    {
        InternalControlPropertiesSection *sourceSection = sourceProperties == nullptr ? nullptr : sourceProperties->GetInternalControlPropertiesSection(i);
        InternalControlPropertiesSection *section = new InternalControlPropertiesSection(control, i, sourceSection, copyType);
        section->SetParent(this);
        internalControlProperties.push_back(section);
    }
}
