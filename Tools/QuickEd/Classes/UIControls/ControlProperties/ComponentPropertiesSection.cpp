//
//  ComponentPropertiesSection.cpp
//  QuickEd
//
//  Created by Dmitry Belsky on 30.11.14.
//
//

#include "ComponentPropertiesSection.h"

#include "UI/UIComponent.h"
#include "ValueProperty.h"

using namespace DAVA;

ComponentPropertiesSection::ComponentPropertiesSection(UIControl *control, const DAVA::String &componentName, const ComponentPropertiesSection *sourceSection)
    :control(NULL)
    , component(NULL)
{
    this->control = SafeRetain(control);

    if (sourceSection != NULL)
    {
        component = sourceSection->GetComponent()->Clone();
    }
    else
    {
        int typeVal = - 1;
        if (GlobalEnumMap<UIComponent::eType>::Instance()->ToValue(componentName.c_str(), typeVal))
        {
            component = UIComponent::CreateByType((UIComponent::eType) typeVal);
        }
        else
        {
            DVASSERT(false);
        }
    }

    if (component)
    {
        control->PutComponent(component);
        const InspInfo *insp = component->GetTypeInfo();
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember *member = insp->Member(j);
            ValueProperty *sourceProp = sourceSection == NULL ? NULL : sourceSection->FindProperty(member);
            if (sourceProp && sourceProp->GetValue() != member->Value(component))
                member->SetValue(component, sourceProp->GetValue());
            
            ValueProperty *prop = new ValueProperty(component, member);
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
    else
    {
        DVASSERT(false);
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

String ComponentPropertiesSection::GetName() const
{
    return GlobalEnumMap<UIComponent::eType>::Instance()->ToString(component->GetType());
}

void ComponentPropertiesSection::AddPropertiesToNode(DAVA::YamlNode *node) const
{
    YamlNode *mapNode = YamlNode::CreateMapNode(false);
    for (auto it = children.begin(); it != children.end(); ++it)
        (*it)->AddPropertiesToNode(mapNode);
    if (mapNode->GetCount() > 0)
        node->Add(GetName(), mapNode);
    else
        SafeRelease(mapNode);
}
