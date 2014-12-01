#include "InternalControlPropertiesSection.h"

#include "ValueProperty.h"

using namespace DAVA;

InternalControlPropertiesSection::InternalControlPropertiesSection(DAVA::UIControl *control, int num, const InternalControlPropertiesSection *sourceSection) : control(NULL), internalControl(NULL), internalControlNum(num)
{
    this->control = SafeRetain(control);
    
    internalControl = SafeRetain(control->GetInternalControl(num));
    if (internalControl == NULL && sourceSection != NULL && sourceSection->GetInternalControl() != NULL)
    {
        internalControl = control->CreateInternalControl(num);
        control->SetInternalControl(num, internalControl);
    }
    
    if (internalControl)
    {
        const InspInfo *insp = internalControl->GetTypeInfo();
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember *member = insp->Member(j);
            ValueProperty *sourceProp = sourceSection == NULL ? NULL : sourceSection->FindProperty(member);
            if (sourceProp && sourceProp->GetValue() != member->Value(internalControl))
                member->SetValue(internalControl, sourceProp->GetValue());
            
            ValueProperty *prop = new ValueProperty(internalControl, member);
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

InternalControlPropertiesSection::~InternalControlPropertiesSection()
{
    SafeRelease(internalControl);
    SafeRelease(control);
}

UIControl *InternalControlPropertiesSection::GetInternalControl() const
{
    return internalControl;
}

void InternalControlPropertiesSection::CreateInternalControl()
{
    if (!internalControl)
    {
        internalControl = control->CreateInternalControl(internalControlNum);
        control->SetInternalControl(internalControlNum, internalControl);
        
        const InspInfo *insp = internalControl->GetTypeInfo();
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember *member = insp->Member(j);
            ValueProperty *prop = new ValueProperty(internalControl, member);
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

DAVA::String InternalControlPropertiesSection::GetName() const
{
    return control->GetInternalControlName(internalControlNum) + control->GetInternalControlDescriptions();
}

void InternalControlPropertiesSection::AddPropertiesToNode(YamlNode *node) const
{
    if (internalControl)
    {
        YamlNode *mapNode = YamlNode::CreateMapNode(false);
        for (auto it = children.begin(); it != children.end(); ++it)
            (*it)->AddPropertiesToNode(mapNode);
        if (mapNode->GetCount() > 0)
            node->Add(GetName(), mapNode);
        else
            SafeRelease(mapNode);
    }
}
