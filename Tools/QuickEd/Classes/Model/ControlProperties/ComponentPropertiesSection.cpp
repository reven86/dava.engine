#include "ComponentPropertiesSection.h"

#include "IntrospectionProperty.h"

#include "UI/UIControl.h"
#include "../PackageSerializer.h"

using namespace DAVA;

ComponentPropertiesSection::ComponentPropertiesSection(DAVA::UIControl *aControl, DAVA::UIComponent::eType type, int32 _index, const ComponentPropertiesSection *sourceSection, eCloneType cloneType)
    : SectionProperty("")
    , control(SafeRetain(aControl))
    , component(nullptr)
    , index(_index)
    , prototypeSection(nullptr) // weak
{
    component = UIComponent::CreateByType(type);
    DVASSERT(component);

    if (sourceSection && cloneType == CT_INHERIT)
    {
        prototypeSection = sourceSection; // weak
    }

    RefreshName();

    const InspInfo *insp = component->GetTypeInfo();
    for (int j = 0; j < insp->MembersCount(); j++)
    {
        const InspMember *member = insp->Member(j);
        
        const ValueProperty *sourceProp = sourceSection == NULL ? NULL : sourceSection->FindProperty(member);
        ValueProperty *prop = new IntrospectionProperty(component, member, dynamic_cast<const IntrospectionProperty *>(sourceProp), cloneType);
        AddProperty(prop);
        SafeRelease(prop);
    }
}

ComponentPropertiesSection::~ComponentPropertiesSection()
{
    SafeRelease(control);
    SafeRelease(component);
    prototypeSection = nullptr; // weak
}

UIComponent *ComponentPropertiesSection::GetComponent() const
{
    return component;
}

DAVA::uint32 ComponentPropertiesSection::GetComponentType() const
{
    return component->GetType();
}

bool ComponentPropertiesSection::HasChanges() const
{
    return SectionProperty::HasChanges();
}

uint32 ComponentPropertiesSection::GetFlags() const
{
    bool readOnly = IsReadOnly();
    
    uint32 flags = 0;
    
    if (!readOnly && prototypeSection == nullptr)
        flags |= EF_CAN_REMOVE;
    
    if (prototypeSection)
        flags |= EF_INHERITED;
    
    return flags;
}

void ComponentPropertiesSection::InstallComponent()
{
    if (control->GetComponent(component->GetType(), 0) != component)
    {
        control->InsertComponentAt(component, index);
    }
}

void ComponentPropertiesSection::UninstallComponent()
{
    UIComponent *installedComponent = control->GetComponent(component->GetType(), index);
    if (installedComponent)
    {
        DVASSERT(installedComponent == component);
        control->RemoveComponent(component);
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
        
        Logger::Debug("Refresh %s, %d, %d", control->GetName().c_str(), component->GetType(), index);

        RefreshName();
    }
}

void ComponentPropertiesSection::Serialize(PackageSerializer *serializer) const
{
    if (HasChanges() || (GetFlags() & AbstractProperty::EF_INHERITED) == 0)
    {
        String name = GetComponentName();
        if (UIComponent::IsMultiple(component->GetType()))
            name += Format("%d", index);
        
        serializer->BeginMap(name);
        
        for (const auto child : children)
            child->Serialize(serializer);
        
        serializer->EndMap();
    }
}

String ComponentPropertiesSection::GetComponentName() const
{
    return GlobalEnumMap<UIComponent::eType>::Instance()->ToString(component->GetType());
}

void ComponentPropertiesSection::RefreshName()
{
    name = GetComponentName();
    if (UIComponent::IsMultiple(component->GetType()))
        name += Format(" [%d]", index);
}
