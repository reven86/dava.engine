#include "PropertiesRoot.h"

#include "ControlPropertiesSection.h"
#include "BackgroundPropertiesSection.h"
#include "InternalControlPropertiesSection.h"
#include "ValueProperty.h"
#include "LocalizedTextValueProperty.h"

using namespace DAVA;

PropertiesRoot::PropertiesRoot(UIControl *control)
{
    MakeControlPropertiesSection(control, control->GetTypeInfo(), NULL);
    MakeBackgroundPropertiesSection(control, NULL);
    MakeInternalControlPropertiesSection(control, NULL);
}

PropertiesRoot::PropertiesRoot(UIControl *control, const PropertiesRoot *sourceProperties)
{
    MakeControlPropertiesSection(control, control->GetTypeInfo(), sourceProperties);
    MakeBackgroundPropertiesSection(control, sourceProperties);
    MakeInternalControlPropertiesSection(control, sourceProperties);
}

PropertiesRoot::~PropertiesRoot()
{
    for (auto it = controlProperties.begin(); it != controlProperties.end(); ++it)
    {
        (*it)->SetParent(NULL);
        (*it)->Release();
    }
    controlProperties.clear();

    for (auto it = backgroundProperties.begin(); it != backgroundProperties.end(); ++it)
    {
        (*it)->SetParent(NULL);
        (*it)->Release();
    }
    backgroundProperties.clear();

    for (auto it = internalControlProperties.begin(); it != internalControlProperties.end(); ++it)
    {
        (*it)->SetParent(NULL);
        (*it)->Release();
    }
    internalControlProperties.clear();
}

int PropertiesRoot::GetCount() const
{
    return (int) (controlProperties.size() + backgroundProperties.size() + internalControlProperties.size());
}

BaseProperty *PropertiesRoot::GetProperty(int index) const
{
    if (index < (int) controlProperties.size())
        return controlProperties[index];
    index -= controlProperties.size();
    
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
    return NULL;
}

BackgroundPropertiesSection *PropertiesRoot::GetBackgroundPropertiesSection(int num) const
{
    if (0 <= num && num < (int) backgroundProperties.size())
        return backgroundProperties[num];
    return NULL;
}

InternalControlPropertiesSection *PropertiesRoot::GetInternalControlPropertiesSection(int num) const
{
    if (0 <= num && num < (int) internalControlProperties.size())
        return internalControlProperties[num];
    return NULL;
}

String PropertiesRoot::GetName() const {
    return "ROOT";
}

BaseProperty::ePropertyType PropertiesRoot::GetType() const {
    return TYPE_HEADER;
}

void PropertiesRoot::MakeControlPropertiesSection(DAVA::UIControl *control, const DAVA::InspInfo *typeInfo, const PropertiesRoot *sourceProperties)
{
    const InspInfo *baseInfo = typeInfo->BaseInfo();
    if (baseInfo)
        MakeControlPropertiesSection(control, baseInfo, sourceProperties);
    
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
        ControlPropertiesSection *sourceSection = sourceProperties == NULL ? NULL : sourceProperties->GetControlPropertiesSection(typeInfo->Name());
        ControlPropertiesSection *section = new ControlPropertiesSection(control, typeInfo, sourceSection);
        section->SetParent(this);
        controlProperties.push_back(section);
    }
}

void PropertiesRoot::AddPropertiesToNode(YamlNode *node) const
{
    for (auto it = controlProperties.begin(); it != controlProperties.end(); ++it)
        (*it)->AddPropertiesToNode(node);

    YamlNode *componentsNode = YamlNode::CreateMapNode(false);
    for (auto it = backgroundProperties.begin(); it != backgroundProperties.end(); ++it)
        (*it)->AddPropertiesToNode(componentsNode);
    for (auto it = internalControlProperties.begin(); it != internalControlProperties.end(); ++it)
        (*it)->AddPropertiesToNode(componentsNode);
    if (componentsNode->GetCount() > 0)
        node->Add("components", componentsNode);
    else
        SafeRelease(componentsNode);
}

void PropertiesRoot::MakeBackgroundPropertiesSection(DAVA::UIControl *control, const PropertiesRoot *sourceProperties)
{
    for (int i = 0; i < control->GetBackgroundComponentsCount(); i++)
    {
        BackgroundPropertiesSection *sourceSection = sourceProperties == NULL ? NULL : sourceProperties->GetBackgroundPropertiesSection(i);
        BackgroundPropertiesSection *section = new BackgroundPropertiesSection(control, i, sourceSection);
        section->SetParent(this);
        backgroundProperties.push_back(section);
    }
}

void PropertiesRoot::MakeInternalControlPropertiesSection(DAVA::UIControl *control, const PropertiesRoot *sourceProperties)
{
    for (int i = 0; i < control->GetInternalControlsCount(); i++)
    {
        InternalControlPropertiesSection *sourceSection = sourceProperties == NULL ? NULL : sourceProperties->GetInternalControlPropertiesSection(i);
        InternalControlPropertiesSection *section = new InternalControlPropertiesSection(control, i, sourceSection);
        section->SetParent(this);
        internalControlProperties.push_back(section);
    }
}
