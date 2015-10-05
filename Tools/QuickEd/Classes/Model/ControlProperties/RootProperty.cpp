/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "RootProperty.h"

#include "PropertyVisitor.h"

#include "ControlPropertiesSection.h"
#include "ComponentPropertiesSection.h"

#include "BackgroundPropertiesSection.h"
#include "InternalControlPropertiesSection.h"

#include "PropertyListener.h"
#include "ValueProperty.h"

#include "NameProperty.h"
#include "PrototypeNameProperty.h"
#include "ClassProperty.h"
#include "CustomClassProperty.h"

#include "../PackageHierarchy/ControlNode.h"
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
            int32 index = section->GetComponentIndex();
            ScopedPtr<ComponentPropertiesSection> newSection(new ComponentPropertiesSection(node->GetControl(), type, index, section, cloneType));
            AddComponentPropertiesSection(newSection);
        }
    }
}

RootProperty::~RootProperty()
{
    node = nullptr; // don't release, just weak ptr
    
    SafeRelease(classProperty);
    SafeRelease(customClassProperty);
    SafeRelease(prototypeProperty);
    SafeRelease(nameProperty);
    DVASSERT(baseProperties.size() == 4);
    baseProperties.clear();

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


    for (ComponentPropertiesSection *section : componentProperties)
    {
        section->SetParent(nullptr);
        section->Release();
    }
    componentProperties.clear();

    listeners.clear();
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

DAVA::int32 RootProperty::GetControlPropertiesSectionsCount() const
{
    return (int32) controlProperties.size();
}

ControlPropertiesSection *RootProperty::GetControlPropertiesSection(DAVA::int32 index) const
{
    if (index >= 0 && index < static_cast<DAVA::int32>(controlProperties.size()))
    {
        return controlProperties[index];
    }
    else
    {
        DVASSERT(false);
        return nullptr;
    }
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
    if (IsReadOnly())
        return false;
    
    if (UIComponent::IsMultiple(componentType))
        return true;
    
    if (FindComponentPropertiesSection(componentType, 0) == nullptr)
        return true;
    
    return false;
}

bool RootProperty::CanRemoveComponent(DAVA::uint32 componentType) const
{
    return !IsReadOnly() && FindComponentPropertiesSection(componentType, 0) != nullptr; // TODO
}

const Vector<ComponentPropertiesSection*> &RootProperty::GetComponents() const
{
    return componentProperties;
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
        return GetComponentAbsIndex(section->GetComponentType(), section->GetComponentIndex()) + offset;
    }
}

ComponentPropertiesSection *RootProperty::FindComponentPropertiesSection(DAVA::uint32 componentType, DAVA::uint32 componentIndex) const
{
    int32 index = 0;
    for (ComponentPropertiesSection *section : componentProperties)
    {
        if (section->GetComponent()->GetType() == componentType)
        {
            if (componentIndex == index)
                return section;
            
            index++;
        }
    }
    return nullptr;
}

ComponentPropertiesSection *RootProperty::AddComponentPropertiesSection(DAVA::uint32 componentType)
{
    uint32 index = 0;
    
    for (ComponentPropertiesSection *s : componentProperties)
    {
        if (s->GetComponentType() == componentType)
            index++;
    }
    
    ComponentPropertiesSection *section = new ComponentPropertiesSection(node->GetControl(), (UIComponent::eType) componentType, index, nullptr, CT_INHERIT);
    AddComponentPropertiesSection(section);
    section->Release();
    return section;
}

void RootProperty::AddComponentPropertiesSection(ComponentPropertiesSection *section)
{
    uint32 componentType = section->GetComponentType();
    if (UIComponent::IsMultiple(componentType) || FindComponentPropertiesSection(componentType, 0) == nullptr)
    {
        int32 index = GetComponentAbsIndex(componentType, section->GetComponentIndex());
        
        int32 globalIndex = GetIndexOfCompoentPropertiesSection(section);
        for (PropertyListener *listener : listeners)
            listener->ComponentPropertiesWillBeAdded(this, section, globalIndex);

        componentProperties.insert(componentProperties.begin() + index, SafeRetain(section));
        DVASSERT(section->GetParent() == nullptr);
        section->SetParent(this);
        section->InstallComponent();

        for (PropertyListener *listener : listeners)
            listener->ComponentPropertiesWasAdded(this, section, globalIndex);
        
        RefreshComponentIndices();
    }
    else
    {
        DVASSERT(false);
    }
}

void RootProperty::RemoveComponentPropertiesSection(DAVA::uint32 componentType, DAVA::uint32 componentIndex)
{
    ComponentPropertiesSection *section = FindComponentPropertiesSection(componentType, componentIndex);
    if (section)
    {
        RemoveComponentPropertiesSection(section);
    }
}

void RootProperty::RemoveComponentPropertiesSection(ComponentPropertiesSection *section)
{
    uint32 componentType = section->GetComponentType();
    
    if (FindComponentPropertiesSection(componentType, section->GetComponentIndex()) == section)
    {
        int index = GetIndexOfCompoentPropertiesSection(section);
        for (PropertyListener *listener : listeners)
            listener->ComponentPropertiesWillBeRemoved(this, section, index);

        auto it = std::find(componentProperties.begin(), componentProperties.end(), section);
        if (it != componentProperties.end())
        {
            componentProperties.erase(it);

            DVASSERT(section->GetParent() == this);
            section->SetParent(nullptr);
            section->UninstallComponent();
            section->Release();
        }
        
        for (PropertyListener *listener : listeners)
            listener->ComponentPropertiesWasRemoved(this, section, index);
        
        RefreshComponentIndices();
    }
    else
    {
        DVASSERT(false);
    }
}

void RootProperty::AttachPrototypeComponent(ComponentPropertiesSection *section, ComponentPropertiesSection *prototypeSection)
{
    section->AttachPrototypeSection(prototypeSection);
}

void RootProperty::DetachPrototypeComponent(ComponentPropertiesSection *section, ComponentPropertiesSection *prototypeSection)
{
    section->DetachPrototypeSection(prototypeSection);
}

const Vector<BackgroundPropertiesSection*> &RootProperty::GetBackgroundProperties() const
{
    return backgroundProperties;
}

BackgroundPropertiesSection *RootProperty::GetBackgroundPropertiesSection(int num) const
{
    if (0 <= num && num < (int) backgroundProperties.size())
        return backgroundProperties[num];
    return nullptr;
}

const DAVA::Vector<InternalControlPropertiesSection*> &RootProperty::GetInternalControlProperties() const
{
    return internalControlProperties;
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

void RootProperty::RefreshProperty(AbstractProperty *property, DAVA::int32 refreshFlags)
{
    property->Refresh(refreshFlags);

    for (PropertyListener *listener : listeners)
        listener->PropertyChanged(property);
}

AbstractProperty* RootProperty::FindPropertyByName(const String& name) const
{
    int propertiesCount = GetCount();
    for (int index = 0; index < propertiesCount; ++index)
    {
        AbstractProperty* rootProperty = GetProperty(index);
        if (nullptr != rootProperty)
        {
            int sectionCount = rootProperty->GetCount();
            for (int prop = 0; prop < sectionCount; ++prop)
            {
                AbstractProperty* valueProperty = rootProperty->GetProperty(prop);
                if (nullptr != valueProperty && valueProperty->GetName() == name)
                {
                    return valueProperty;
                }
            }
        }
    }
    return nullptr;
}

void RootProperty::Refresh(DAVA::int32 refreshFlags)
{
    for (int32 i = 0; i < GetCount(); i++)
        GetProperty(i)->Refresh(refreshFlags);
}

void RootProperty::Accept(PropertyVisitor *visitor)
{
    visitor->VisitRootProperty(this);
}

bool RootProperty::IsReadOnly() const
{
    return !node->IsEditingSupported();
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
    NameProperty *sourceNameProperty = sourceProperties == nullptr ? nullptr : sourceProperties->GetNameProperty();
    nameProperty = new NameProperty(node, sourceNameProperty, cloneType);
    baseProperties.push_back(nameProperty);
    
    PrototypeNameProperty *sourcePrototypeProperty = sourceProperties == nullptr ? nullptr : sourceProperties->GetPrototypeProperty();
    prototypeProperty = new PrototypeNameProperty(node, sourcePrototypeProperty, cloneType);
    baseProperties.push_back(prototypeProperty);
    
    classProperty = new ClassProperty(node);
    baseProperties.push_back(classProperty);
    
    CustomClassProperty *sourceCustomClassProperty = sourceProperties == nullptr ? nullptr : sourceProperties->GetCustomClassProperty();
    customClassProperty = new CustomClassProperty(node, sourceCustomClassProperty, cloneType);
    baseProperties.push_back(customClassProperty);

    for (ValueProperty *prop : baseProperties)
        prop->SetParent(this);
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
        ControlPropertiesSection *sourceSection = sourceProperties == nullptr ? nullptr : sourceProperties->GetControlPropertiesSection(typeInfo->Name().c_str());
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

uint32 RootProperty::GetComponentAbsIndex(DAVA::uint32 componentType, DAVA::uint32 index) const
{
    uint32 i = 0;
    for (ComponentPropertiesSection *section : componentProperties)
    {
        if (section->GetComponentType() >= componentType)
        {
            return index + i;
        }
        i++;
    }
    DVASSERT(index == 0);
    return (uint32) componentProperties.size();
}

void RootProperty::RefreshComponentIndices()
{
    for (ComponentPropertiesSection *section : componentProperties)
    {
        section->RefreshIndex();
        
        for (PropertyListener *listener : listeners)
            listener->PropertyChanged(section);
    }
}
