#include "ComponentPropertiesSection.h"

#include "ValueProperty.h"

#include "UI/UIControl.h"
#include "../PackageSerializer.h"

using namespace DAVA;

ComponentPropertiesSection::ComponentPropertiesSection(UIControl *control, UIComponent::eType type, const ComponentPropertiesSection *sourceSection, eCopyType copyType)
    : control(nullptr), component(nullptr)
{
    this->control = SafeRetain(control);
    
    component = UIComponent::CreateByType(type);
    DVASSERT(component);

    control->PutComponent(component);
    
    if (component)
    {
        const InspInfo *insp = component->GetTypeInfo();
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember *member = insp->Member(j);
            
            ValueProperty *sourceProp = sourceSection == NULL ? NULL : sourceSection->FindProperty(member);
            ValueProperty *prop = new ValueProperty(component, member, sourceProp, copyType);
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

DAVA::String ComponentPropertiesSection::GetName() const
{
    return String(GlobalEnumMap<UIComponent::eType>::Instance()->ToString(component->GetType()));
}

bool ComponentPropertiesSection::CanRemove() const
{
    return !IsReadOnly();
}

bool ComponentPropertiesSection::HasChanges() const
{
    return PropertiesSection::HasChanges();
}

void ComponentPropertiesSection::Serialize(PackageSerializer *serializer) const
{
    serializer->BeginMap(GetName());
    
    for (const auto child : children)
        child->Serialize(serializer);
    
    serializer->EndMap();
}
