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
    
    for (ComponentPropertiesSection *section : sourceProperties->componentProperties)
    {
        UIComponent::eType type = (UIComponent::eType) section->GetComponent()->GetType();
        ScopedPtr<ComponentPropertiesSection> newSection(new ComponentPropertiesSection(control, type, section, copyType));
        AddComponentPropertiesSection(newSection);
    }
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

int32 PropertiesRoot::GetIndexOfCompoentPropertiesSection(ComponentPropertiesSection *section)
{
    int32 offset = (int32) controlProperties.size();
    auto it = std::find(componentProperties.begin(), componentProperties.end(), section);
    if (it != componentProperties.end())
    {
        return (it - componentProperties.begin()) + offset;
    }
    else
    {
        int32 componentsCount = (int32) componentProperties.size();
        for (int32 i = 0; i < componentsCount; i++)
        {
            if (componentProperties[i]->GetType() > section->GetType())
                return i + offset;
        }
        return componentsCount + offset;
    }
}

ComponentPropertiesSection *PropertiesRoot::FindComponentPropertiesSection(DAVA::uint32 componentType)
{
    for (ComponentPropertiesSection *section : componentProperties)
    {
        if (section->GetComponent()->GetType() == componentType)
        {
            return section;
        }
    }
    return nullptr;
}

ComponentPropertiesSection *PropertiesRoot::AddComponentPropertiesSection(DAVA::uint32 componentType)
{
    ComponentPropertiesSection *section = new ComponentPropertiesSection(control, (UIComponent::eType) componentType, nullptr, COPY_VALUES);
    AddComponentPropertiesSection(section);
    section->Release();
    return section;
}

void PropertiesRoot::AddComponentPropertiesSection(ComponentPropertiesSection *section)
{
    componentProperties.push_back(SafeRetain(section));
    
    DVASSERT(control->HasComponent(section->GetComponent()->GetType()) == false);
    control->PutComponent(section->GetComponent());
    std::stable_sort(componentProperties.begin(), componentProperties.end(), [](ComponentPropertiesSection * left, ComponentPropertiesSection * right) {
        return left->GetComponent()->GetType() < right->GetComponent()->GetType();
    });

}

void PropertiesRoot::RemoveComponentPropertiesSection(DAVA::uint32 componentType)
{
    ComponentPropertiesSection *section = FindComponentPropertiesSection(componentType);
    if (section)
    {
        RemoveComponentPropertiesSection(section);
    }
}

void PropertiesRoot::RemoveComponentPropertiesSection(ComponentPropertiesSection *section)
{
    auto it = std::find(componentProperties.begin(), componentProperties.end(), section);
    if (it != componentProperties.end())
    {
        DVASSERT(control->GetComponent(section->GetComponent()->GetType()) == section->GetComponent());
        control->RemoveComponent(section->GetComponent());
        componentProperties.erase(it);
        section->Release();
    }
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
