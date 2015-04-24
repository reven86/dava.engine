#include "RootProperty.h"

#include "ControlPropertiesSection.h"
#include "ComponentPropertiesSection.h"

#include "BackgroundPropertiesSection.h"
#include "InternalControlPropertiesSection.h"

#include "PropertyListener.h"
#include "ValueProperty.h"

#include "Model/PackageSerializer.h"
#include "StringProperty.h"

#include "../PackageHierarchy/ControlNode.h"

#include "Base/FunctionTraits.h"
#include "UI/UIControl.h"

using namespace DAVA;

RootProperty::RootProperty(ControlNode *_node, const RootProperty *sourceProperties, eCloneType cloneType)
    : node(_node)
    , classProperty(nullptr)
    , customClassProperty(nullptr)
    , prototypeProperty(nullptr)
    , nameProperty(nullptr)
{
    AddBaseProperties(node->GetControl(), sourceProperties, cloneType);
    MakeControlPropertiesSection(node->GetControl(), node->GetControl()->GetTypeInfo(), sourceProperties, cloneType);
    MakeBackgroundPropertiesSection(node->GetControl(), sourceProperties, cloneType);
    MakeInternalControlPropertiesSection(node->GetControl(), sourceProperties, cloneType);

    if (sourceProperties)
    {
        for (ComponentPropertiesSection *section : sourceProperties->componentProperties)
        {
            UIComponent::eType type = (UIComponent::eType) section->GetComponent()->GetType();
            ScopedPtr<ComponentPropertiesSection> newSection(new ComponentPropertiesSection(node->GetControl(), type, section, cloneType));
            AddComponentPropertiesSection(newSection);
        }
    }
}

RootProperty::~RootProperty()
{
    node = nullptr; // don't release, just week ptr
    
    SafeRelease(classProperty);
    SafeRelease(customClassProperty);
    SafeRelease(prototypeProperty);
    SafeRelease(nameProperty);

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

int RootProperty::GetCount() const
{
    return (int)(baseProperties.size() + controlProperties.size() + componentProperties.size() + backgroundProperties.size() + internalControlProperties.size());
}

AbstractProperty *RootProperty::GetProperty(int index) const
{
    if (index < (int)baseProperties.size())
        return baseProperties[index];
    index -= baseProperties.size();

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

ControlPropertiesSection *RootProperty::GetControlPropertiesSection(const DAVA::String &name) const
{
    for (auto it = controlProperties.begin(); it != controlProperties.end(); ++it)
    {
        if ((*it)->GetName() == name)
            return *it;
    }
    return nullptr;
}

bool RootProperty::CanAddComponent(DAVA::uint32 componentType) const
{
    return !IsReadOnly() && FindComponentPropertiesSection(componentType) == nullptr;
}

bool RootProperty::CanRemoveComponent(DAVA::uint32 componentType) const
{
    return !IsReadOnly() && FindComponentPropertiesSection(componentType) != nullptr;
}

int32 RootProperty::GetIndexOfCompoentPropertiesSection(ComponentPropertiesSection *section) const
{
    int32 offset = controlProperties.size() + baseProperties.size();
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

ComponentPropertiesSection *RootProperty::FindComponentPropertiesSection(DAVA::uint32 componentType) const
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

ComponentPropertiesSection *RootProperty::AddComponentPropertiesSection(DAVA::uint32 componentType)
{
    ComponentPropertiesSection *section = new ComponentPropertiesSection(node->GetControl(), (UIComponent::eType) componentType, nullptr, CT_INHERIT);
    AddComponentPropertiesSection(section);
    section->Release();
    return section;
}

void RootProperty::AddComponentPropertiesSection(ComponentPropertiesSection *section)
{
    if (FindComponentPropertiesSection(section->GetComponentType()) == nullptr)
    {
        int index = GetIndexOfCompoentPropertiesSection(section);
        for (PropertyListener *listener : listeners)
            listener->ComponentPropertiesWillBeAdded(this, section, index);

        componentProperties.push_back(SafeRetain(section));
        section->InstallComponent();

        std::stable_sort(componentProperties.begin(), componentProperties.end(), [](ComponentPropertiesSection * left, ComponentPropertiesSection * right) {
            return left->GetComponent()->GetType() < right->GetComponent()->GetType();
        });

        for (PropertyListener *listener : listeners)
            listener->ComponentPropertiesWasAdded(this, section, index);
    }
    else
    {
        DVASSERT(false);
    }
}

void RootProperty::RemoveComponentPropertiesSection(DAVA::uint32 componentType)
{
    ComponentPropertiesSection *section = FindComponentPropertiesSection(componentType);
    if (section)
    {
        RemoveComponentPropertiesSection(section);
    }
}

void RootProperty::RemoveComponentPropertiesSection(ComponentPropertiesSection *section)
{
    uint32 componentType = section->GetComponentType();
    
    if (FindComponentPropertiesSection(componentType) == section)
    {
        int index = GetIndexOfCompoentPropertiesSection(section);
        for (PropertyListener *listener : listeners)
            listener->ComponentPropertiesWillBeRemoved(this, section, index);

        auto it = std::find(componentProperties.begin(), componentProperties.end(), section);
        if (it != componentProperties.end())
        {
            componentProperties.erase(it);

            section->UninstallComponent();
            section->Release();
        }
        
        for (PropertyListener *listener : listeners)
            listener->ComponentPropertiesWasRemoved(this, section, index);
    }
    else
    {
        DVASSERT(false);
    }
}

BackgroundPropertiesSection *RootProperty::GetBackgroundPropertiesSection(int num) const
{
    if (0 <= num && num < (int) backgroundProperties.size())
        return backgroundProperties[num];
    return nullptr;
}

InternalControlPropertiesSection *RootProperty::GetInternalControlPropertiesSection(int num) const
{
    if (0 <= num && num < (int) internalControlProperties.size())
        return internalControlProperties[num];
    return nullptr;
}

void RootProperty::AddListener(PropertyListener *listener)
{
    listeners.push_back(listener);
}

void RootProperty::RemoveListener(PropertyListener *listener)
{
    auto it = std::find(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end())
    {
        listeners.erase(it);
    }
    else
    {
        DVASSERT(false);
    }
}

void RootProperty::SetProperty(AbstractProperty *property, const DAVA::VariantType &newValue)
{
    property->SetValue(newValue);
    
    for (PropertyListener *listener : listeners)
        listener->PropertyChanged(property);
}

void RootProperty::SetDefaultProperty(AbstractProperty *property, const DAVA::VariantType &newValue)
{
    property->SetDefaultValue(newValue);

    for (PropertyListener *listener : listeners)
        listener->PropertyChanged(property);
}

void RootProperty::ResetProperty(AbstractProperty *property)
{
    property->ResetValue();

    for (PropertyListener *listener : listeners)
        listener->PropertyChanged(property);
}

void RootProperty::Serialize(PackageSerializer *serializer) const
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

const DAVA::String & RootProperty::GetName() const
{
    static String rootName = "ROOT";
    return rootName;
}

AbstractProperty::ePropertyType RootProperty::GetType() const
{
    return TYPE_HEADER;
}

void RootProperty::AddBaseProperties(DAVA::UIControl *control, const RootProperty *sourceProperties, eCloneType cloneType)
{
    ValueProperty *sourceClassProperty = sourceProperties == nullptr ? nullptr : sourceProperties->GetClassProperty();
    classProperty = new StringProperty("Class", control, DAVA::MakeFunction(&DAVA::UIControl::GetControlClassName), NULL, dynamic_cast<StringProperty*>(sourceClassProperty), cloneType);
    baseProperties.push_back(classProperty);

    ValueProperty *sourceCustomClassProperty = sourceProperties == nullptr ? nullptr : sourceProperties->GetCustomClassProperty();
    customClassProperty = new StringProperty("Custom class", control, DAVA::MakeFunction(&DAVA::UIControl::GetCustomControlClassName), DAVA::MakeFunction(&DAVA::UIControl::SetCustomControlClassName), dynamic_cast<StringProperty*>(sourceCustomClassProperty), cloneType);
    baseProperties.push_back(customClassProperty);

    ValueProperty *sourcePrototypeProperty = sourceProperties == nullptr ? nullptr : sourceProperties->GetPrototypeProperty();
    prototypeProperty = new StringProperty("Prototype", control, DAVA::MakeFunction(&DAVA::UIControl::GetCustomControlClassName), NULL, dynamic_cast<StringProperty*>(sourcePrototypeProperty), cloneType);
    baseProperties.push_back(prototypeProperty);

    ValueProperty *sourceNameProperty = sourceProperties == nullptr ? nullptr : sourceProperties->GetNameProperty();
    nameProperty = new StringProperty("Name", control, DAVA::MakeFunction(&DAVA::UIControl::GetName), DAVA::MakeFunction(&DAVA::UIControl::SetName), dynamic_cast<StringProperty*>(sourceNameProperty), cloneType);
    baseProperties.push_back(nameProperty);
}

void RootProperty::MakeControlPropertiesSection(DAVA::UIControl *control, const DAVA::InspInfo *typeInfo, const RootProperty *sourceProperties, eCloneType copyType)
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

void RootProperty::MakeBackgroundPropertiesSection(DAVA::UIControl *control, const RootProperty *sourceProperties, eCloneType copyType)
{
    for (int i = 0; i < control->GetBackgroundComponentsCount(); i++)
    {
        BackgroundPropertiesSection *sourceSection = sourceProperties == nullptr ? nullptr : sourceProperties->GetBackgroundPropertiesSection(i);
        BackgroundPropertiesSection *section = new BackgroundPropertiesSection(control, i, sourceSection, copyType);
        section->SetParent(this);
        backgroundProperties.push_back(section);
    }
}

void RootProperty::MakeInternalControlPropertiesSection(DAVA::UIControl *control, const RootProperty *sourceProperties, eCloneType copyType)
{
    for (int i = 0; i < control->GetInternalControlsCount(); i++)
    {
        InternalControlPropertiesSection *sourceSection = sourceProperties == nullptr ? nullptr : sourceProperties->GetInternalControlPropertiesSection(i);
        InternalControlPropertiesSection *section = new InternalControlPropertiesSection(control, i, sourceSection, copyType);
        section->SetParent(this);
        internalControlProperties.push_back(section);
    }
}
