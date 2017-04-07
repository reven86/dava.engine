#include "RootProperty.h"

#include "PropertyVisitor.h"

#include "ControlPropertiesSection.h"
#include "ComponentPropertiesSection.h"

#include "PropertyListener.h"
#include "ValueProperty.h"

#include "NameProperty.h"
#include "PrototypeNameProperty.h"
#include "ClassProperty.h"
#include "CustomClassProperty.h"
#include "VisibleValueProperty.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "UI/UIControl.h"

using namespace DAVA;

RootProperty::RootProperty(ControlNode* _node, const RootProperty* sourceProperties, eCloneType cloneType)
    : node(_node)
    , classProperty(nullptr)
    , customClassProperty(nullptr)
    , prototypeProperty(nullptr)
    , nameProperty(nullptr)
{
    AddBaseProperties(node->GetControl(), sourceProperties, cloneType);

    UIControl* control = node->GetControl();
    Reflection controlRef = Reflection::Create(&control);
    Vector<Reflection::Field> fields = controlRef.GetFields();
    MakeControlPropertiesSection(node->GetControl(), ReflectedTypeDB::GetByPointer(control)->GetType(), fields, sourceProperties, cloneType);

    if (sourceProperties)
    {
        for (ComponentPropertiesSection* section : sourceProperties->componentProperties)
        {
            UIComponent::eType type = static_cast<UIComponent::eType>(section->GetComponent()->GetType());
            int32 index = section->GetComponentIndex();
            ScopedPtr<ComponentPropertiesSection> newSection(new ComponentPropertiesSection(node->GetControl(), type, index, section, cloneType));
            AddComponentPropertiesSection(newSection);
        }
    }
    visibleProperty = DynamicTypeCheck<VisibleValueProperty*>(FindPropertyByName("visible"));
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

    visibleProperty = nullptr;

    for (ControlPropertiesSection* section : controlProperties)
    {
        section->SetParent(nullptr);
        section->Release();
    }
    controlProperties.clear();

    for (ComponentPropertiesSection* section : componentProperties)
    {
        section->SetParent(nullptr);
        section->Release();
    }
    componentProperties.clear();

    listeners.clear();
}

uint32 RootProperty::GetCount() const
{
    return static_cast<uint32>(baseProperties.size() + controlProperties.size() + componentProperties.size());
}

AbstractProperty* RootProperty::GetProperty(int index) const
{
    DVASSERT(index >= 0);

    if (index < static_cast<int>(baseProperties.size()))
        return baseProperties[index];
    index -= static_cast<int>(baseProperties.size());

    if (index < static_cast<int>(controlProperties.size()))
        return controlProperties[index];
    index -= static_cast<int>(controlProperties.size());

    DVASSERT(index < static_cast<int>(componentProperties.size()));
    return componentProperties[index];
}

DAVA::int32 RootProperty::GetControlPropertiesSectionsCount() const
{
    return static_cast<int32>(controlProperties.size());
}

ControlPropertiesSection* RootProperty::GetControlPropertiesSection(DAVA::int32 index) const
{
    if (index >= 0 && index < static_cast<int32>(controlProperties.size()))
    {
        return controlProperties[index];
    }
    else
    {
        DVASSERT(false);
        return nullptr;
    }
}

ControlPropertiesSection* RootProperty::GetControlPropertiesSection(const DAVA::String& name) const
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

const Vector<ComponentPropertiesSection*>& RootProperty::GetComponents() const
{
    return componentProperties;
}

int32 RootProperty::GetIndexOfCompoentPropertiesSection(ComponentPropertiesSection* section) const
{
    int32 offset = static_cast<int32>(controlProperties.size() + baseProperties.size());
    auto it = std::find(componentProperties.begin(), componentProperties.end(), section);
    if (it != componentProperties.end())
    {
        return static_cast<int32>(((it - componentProperties.begin()) + offset));
    }
    else
    {
        return GetComponentAbsIndex(section->GetComponentType(), section->GetComponentIndex()) + offset;
    }
}

ComponentPropertiesSection* RootProperty::FindComponentPropertiesSection(DAVA::uint32 componentType, DAVA::uint32 componentIndex) const
{
    int32 index = 0;
    for (ComponentPropertiesSection* section : componentProperties)
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

ComponentPropertiesSection* RootProperty::AddComponentPropertiesSection(DAVA::uint32 componentType)
{
    uint32 index = 0;

    for (ComponentPropertiesSection* s : componentProperties)
    {
        if (s->GetComponentType() == componentType)
            index++;
    }

    ScopedPtr<ComponentPropertiesSection> section(new ComponentPropertiesSection(node->GetControl(), static_cast<UIComponent::eType>(componentType), index, nullptr, CT_INHERIT));
    AddComponentPropertiesSection(section);
    return section;
}

void RootProperty::AddComponentPropertiesSection(ComponentPropertiesSection* section)
{
    uint32 componentType = section->GetComponentType();
    if (UIComponent::IsMultiple(componentType) || FindComponentPropertiesSection(componentType, 0) == nullptr)
    {
        int32 index = GetComponentAbsIndex(componentType, section->GetComponentIndex());

        int32 globalIndex = GetIndexOfCompoentPropertiesSection(section);
        for (PropertyListener* listener : listeners)
            listener->ComponentPropertiesWillBeAdded(this, section, globalIndex);

        componentProperties.insert(componentProperties.begin() + index, SafeRetain(section));
        DVASSERT(section->GetParent() == nullptr);
        section->SetParent(this);
        section->InstallComponent();

        for (PropertyListener* listener : listeners)
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
    ComponentPropertiesSection* section = FindComponentPropertiesSection(componentType, componentIndex);
    if (section)
    {
        RemoveComponentPropertiesSection(section);
    }
}

void RootProperty::RemoveComponentPropertiesSection(ComponentPropertiesSection* section)
{
    uint32 componentType = section->GetComponentType();

    if (FindComponentPropertiesSection(componentType, section->GetComponentIndex()) == section)
    {
        int index = GetIndexOfCompoentPropertiesSection(section);
        for (PropertyListener* listener : listeners)
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

        for (PropertyListener* listener : listeners)
            listener->ComponentPropertiesWasRemoved(this, section, index);

        RefreshComponentIndices();
    }
    else
    {
        DVASSERT(false);
    }
}

void RootProperty::AttachPrototypeComponent(ComponentPropertiesSection* section, ComponentPropertiesSection* prototypeSection)
{
    section->AttachPrototypeSection(prototypeSection);
}

void RootProperty::DetachPrototypeComponent(ComponentPropertiesSection* section, ComponentPropertiesSection* prototypeSection)
{
    section->DetachPrototypeSection(prototypeSection);
}

void RootProperty::AddListener(PropertyListener* listener)
{
    listeners.push_back(listener);
}

void RootProperty::RemoveListener(PropertyListener* listener)
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

void RootProperty::SetProperty(AbstractProperty* property, const DAVA::Any& newValue)
{
    property->SetValue(newValue);

    for (PropertyListener* listener : listeners)
        listener->PropertyChanged(property);
}

void RootProperty::SetDefaultProperty(AbstractProperty* property, const DAVA::Any& newValue)
{
    property->SetDefaultValue(newValue);

    for (PropertyListener* listener : listeners)
        listener->PropertyChanged(property);
}

void RootProperty::ResetProperty(AbstractProperty* property)
{
    property->ResetValue();

    for (PropertyListener* listener : listeners)
        listener->PropertyChanged(property);
}

void RootProperty::RefreshProperty(AbstractProperty* property, DAVA::int32 refreshFlags)
{
    property->Refresh(refreshFlags);

    for (PropertyListener* listener : listeners)
        listener->PropertyChanged(property);
}

void RootProperty::Refresh(DAVA::int32 refreshFlags)
{
    for (int32 i = 0; i < static_cast<int32>(GetCount()); i++)
    {
        GetProperty(i)->Refresh(refreshFlags);
    }
}

void RootProperty::Accept(PropertyVisitor* visitor)
{
    visitor->VisitRootProperty(this);
}

bool RootProperty::IsReadOnly() const
{
    return !node->IsEditingSupported();
}

const DAVA::String& RootProperty::GetName() const
{
    static String rootName = "ROOT";
    return rootName;
}

const DAVA::Type* RootProperty::GetValueType() const
{
    return nullptr;
}

AbstractProperty::ePropertyType RootProperty::GetType() const
{
    return TYPE_HEADER;
}

ControlNode* RootProperty::GetControlNode() const
{
    return node;
}

void RootProperty::AddBaseProperties(DAVA::UIControl* control, const RootProperty* sourceProperties, eCloneType cloneType)
{
    NameProperty* sourceNameProperty = sourceProperties == nullptr ? nullptr : sourceProperties->GetNameProperty();
    nameProperty = new NameProperty(node, sourceNameProperty, cloneType);
    baseProperties.push_back(nameProperty);

    PrototypeNameProperty* sourcePrototypeProperty = sourceProperties == nullptr ? nullptr : sourceProperties->GetPrototypeProperty();
    prototypeProperty = new PrototypeNameProperty(node, sourcePrototypeProperty, cloneType);
    baseProperties.push_back(prototypeProperty);

    classProperty = new ClassProperty(node);
    baseProperties.push_back(classProperty);

    CustomClassProperty* sourceCustomClassProperty = sourceProperties == nullptr ? nullptr : sourceProperties->GetCustomClassProperty();
    customClassProperty = new CustomClassProperty(node, sourceCustomClassProperty, cloneType);
    baseProperties.push_back(customClassProperty);

    for (ValueProperty* prop : baseProperties)
        prop->SetParent(this);
}

void RootProperty::MakeControlPropertiesSection(DAVA::UIControl* control, const DAVA::Type* type, const Vector<Reflection::Field>& fields, const RootProperty* sourceProperties, eCloneType copyType)
{
    const TypeInheritance* inheritance = type->GetInheritance();
    if (type != Type::Instance<UIControl>() && inheritance != nullptr)
    {
        const Vector<TypeInheritance::Info>& baseTypesInfo = inheritance->GetBaseTypes();
        for (const TypeInheritance::Info& baseInfo : baseTypesInfo)
        {
            MakeControlPropertiesSection(control, baseInfo.type, fields, sourceProperties, copyType);
        }
    }

    bool hasProperties = false;
    for (const Reflection::Field& field : fields)
    {
        if (field.inheritFrom->GetType() == type)
        {
            hasProperties = true;
            break;
        }
    }

    if (hasProperties)
    {
        const ReflectedType* rt = ReflectedTypeDB::GetByType(type);
        String sectionName = rt->GetPermanentName();
        ControlPropertiesSection* sourceSection = sourceProperties == nullptr ? nullptr : sourceProperties->GetControlPropertiesSection(sectionName);

        ControlPropertiesSection* section = new ControlPropertiesSection(sectionName, control, type, fields, sourceSection, copyType);
        section->SetParent(this);
        controlProperties.push_back(section);
    }
}

uint32 RootProperty::GetComponentAbsIndex(DAVA::uint32 componentType, DAVA::uint32 index) const
{
    uint32 i = 0;
    for (ComponentPropertiesSection* section : componentProperties)
    {
        if (section->GetComponentType() >= componentType)
        {
            return index + i;
        }
        i++;
    }
    DVASSERT(index == 0);
    return static_cast<uint32>(componentProperties.size());
}

void RootProperty::RefreshComponentIndices()
{
    for (ComponentPropertiesSection* section : componentProperties)
    {
        section->RefreshIndex();

        for (PropertyListener* listener : listeners)
            listener->PropertyChanged(section);
    }
}
