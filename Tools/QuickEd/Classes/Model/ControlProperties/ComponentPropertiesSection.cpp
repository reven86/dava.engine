#include "ComponentPropertiesSection.h"

#include "IntrospectionProperty.h"

#include "UI/UIControl.h"
#include "../PackageSerializer.h"

using namespace DAVA;

ComponentPropertiesSection::ComponentPropertiesSection(DAVA::UIControl *aControl, DAVA::UIComponent::eType type, const ComponentPropertiesSection *sourceSection, eCloneType cloneType)
    : SectionProperty("")
    , control(SafeRetain(aControl))
    , component(nullptr)
    , index(0)
{
    component = UIComponent::CreateByType(type);
    DVASSERT(component);

    if (component)
    {
        name = GetComponentName();

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
}

ComponentPropertiesSection::~ComponentPropertiesSection()
{
    SafeRelease(control);
    SafeRelease(component);
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
    
    if (!readOnly)
        flags |= EF_CAN_REMOVE;
    
    return flags;
}

void ComponentPropertiesSection::InstallComponent()
{
    if (control->GetComponent(component->GetType(), 0) != component)
    {
        control->AddComponent(component);
        
        name = GetComponentName();
        if (UIComponent::IsMultiple(component->GetType()))
            name += Format(" [%d]", index);
    }
}

void ComponentPropertiesSection::UninstallComponent()
{
    UIComponent *installedComponent = control->GetComponent(component->GetType());
    if (installedComponent)
    {
        DVASSERT(installedComponent == component);
        control->RemoveComponent(component);
    }
}

void ComponentPropertiesSection::Serialize(PackageSerializer *serializer) const
{
    String name = GetComponentName();
    if (UIComponent::IsMultiple(component->GetType()))
        name += Format("%d", index);
    
    serializer->BeginMap(name);
    
    for (const auto child : children)
        child->Serialize(serializer);
    
    serializer->EndMap();
}

String ComponentPropertiesSection::GetComponentName() const
{
    return GlobalEnumMap<UIComponent::eType>::Instance()->ToString(component->GetType());
}
