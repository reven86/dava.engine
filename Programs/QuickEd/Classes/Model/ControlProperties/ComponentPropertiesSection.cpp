#include "ComponentPropertiesSection.h"

#include "IntrospectionProperty.h"
#include "PropertyVisitor.h"

#include <UI/UIControl.h>
#include <UI/Layouts/UILayoutIsolationComponent.h>
#include <UI/Layouts/UILayoutSourceRectComponent.h>
#include <UI/Render/UISceneComponent.h>
#include <UI/RichContent/UIRichContentObjectComponent.h>
#include <UI/Scroll/UIScrollComponent.h>
#include <Utils/StringFormat.h>
#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectedTypeDB.h>

using namespace DAVA;

ComponentPropertiesSection::ComponentPropertiesSection(DAVA::UIControl* control_, const DAVA::Type* type_, int32 index_, const ComponentPropertiesSection* sourceSection, eCloneType cloneType)
    : SectionProperty("")
    , control(SafeRetain(control_))
    , component(nullptr)
    , index(index_)
    , prototypeSection(nullptr) // weak
{
    component = control->GetComponent(type_, index_);
    if (component != nullptr)
    {
        SafeRetain(component);
    }
    else
    {
        component = UIComponent::CreateByType(type_);
        componentWasCreated = true;
    }
    DVASSERT(component);

    if (sourceSection && cloneType == CT_INHERIT)
    {
        prototypeSection = sourceSection; // weak
    }

    RefreshName();

    Reflection componentRef = Reflection::Create(&component);
    Vector<Reflection::Field> fields = componentRef.GetFields();
    for (const Reflection::Field& field : fields)
    {
        if (!field.ref.IsReadonly() && nullptr == field.ref.GetMeta<DAVA::M::ReadOnly>())
        {
            String name = field.key.Get<FastName>().c_str();
            const IntrospectionProperty* sourceProp = sourceSection == nullptr ? nullptr : sourceSection->FindChildPropertyByName(name);
            IntrospectionProperty* prop = IntrospectionProperty::Create(component, type_, name.c_str(), field.ref, sourceProp, cloneType);
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

ComponentPropertiesSection::~ComponentPropertiesSection()
{
    SafeRelease(control);
    SafeRelease(component);
    prototypeSection = nullptr; // weak
}

bool ComponentPropertiesSection::IsHiddenComponent(const Type* type)
{
    return (type == Type::Instance<UILayoutIsolationComponent>() ||
            type == Type::Instance<UILayoutSourceRectComponent>() ||
            type == Type::Instance<UIScrollComponent>() ||
            type == Type::Instance<UIRichContentObjectComponent>() ||
            type == Type::Instance<UISceneComponent>());
}

UIComponent* ComponentPropertiesSection::GetComponent() const
{
    return component;
}

const DAVA::Type* ComponentPropertiesSection::GetComponentType() const
{
    return component->GetType();
}

void ComponentPropertiesSection::AttachPrototypeSection(ComponentPropertiesSection* section)
{
    if (prototypeSection == nullptr)
    {
        prototypeSection = section;

        Reflection componentRef = Reflection::Create(&component);
        Vector<Reflection::Field> fields = componentRef.GetFields();

        for (const Reflection::Field& field : fields)
        {
            String name = field.key.Cast<String>();
            ValueProperty* value = FindChildPropertyByName(name);
            ValueProperty* prototypeValue = prototypeSection->FindChildPropertyByName(name);
            if (value != nullptr && prototypeValue != nullptr)
            {
                value->AttachPrototypeProperty(prototypeValue);
            }
            else
            {
                DVASSERT(value == nullptr && prototypeValue == nullptr);
            }
        }
    }
    else
    {
        DVASSERT(false);
    }
}

void ComponentPropertiesSection::DetachPrototypeSection(ComponentPropertiesSection* section)
{
    if (prototypeSection == section)
    {
        prototypeSection = nullptr; // weak
        for (uint32 i = 0; i < GetCount(); i++)
        {
            ValueProperty* value = GetProperty(i);
            if (value->GetPrototypeProperty())
            {
                DVASSERT(value->GetPrototypeProperty()->GetParent() == section);
                value->DetachPrototypeProperty(value->GetPrototypeProperty());
            }
            else
            {
                DVASSERT(false);
            }
        }
    }
    else
    {
        DVASSERT(false);
    }
}

bool ComponentPropertiesSection::HasChanges() const
{
    return SectionProperty::HasChanges() || ((GetFlags() & AbstractProperty::EF_INHERITED) == 0 && componentWasCreated);
}

uint32 ComponentPropertiesSection::GetFlags() const
{
    bool readOnly = IsReadOnly();

    uint32 flags = 0;

    if (!readOnly && prototypeSection == nullptr && componentWasCreated)
    {
        flags |= EF_CAN_REMOVE;
    }

    if (prototypeSection)
    {
        flags |= EF_INHERITED;
    }

    return flags;
}

void ComponentPropertiesSection::InstallComponent()
{
    if (componentWasCreated)
    {
        if (control->GetComponent(component->GetType(), 0) != component)
        {
            control->InsertComponentAt(component, index);
        }
    }
}

void ComponentPropertiesSection::UninstallComponent()
{
    if (componentWasCreated)
    {
        UIComponent* installedComponent = control->GetComponent(component->GetType(), index);
        if (installedComponent)
        {
            DVASSERT(installedComponent == component);
            control->RemoveComponent(component);
        }
    }
}

int32 ComponentPropertiesSection::GetComponentIndex() const
{
    return index;
}

void ComponentPropertiesSection::RefreshIndex()
{
    if (component->GetControl() == control)
    {
        index = control->GetComponentIndex(component);
        RefreshName();
    }
}

void ComponentPropertiesSection::Accept(PropertyVisitor* visitor)
{
    visitor->VisitComponentSection(this);
}

String ComponentPropertiesSection::GetComponentName() const
{
    return ReflectedTypeDB::GetByType(component->GetType())->GetPermanentName();
}

void ComponentPropertiesSection::RefreshName()
{
    name = GetComponentName();
    if (UIComponent::IsMultiple(component->GetType()))
        name += Format(" [%d]", index);
}
