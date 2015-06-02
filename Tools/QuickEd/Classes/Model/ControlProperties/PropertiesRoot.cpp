#include "PropertiesRoot.h"

#include "UI/UIControl.h"

#include "ControlPropertiesSection.h"
#include "BackgroundPropertiesSection.h"
#include "InternalControlPropertiesSection.h"

#include "../PackageSerializer.h"

using namespace DAVA;

PropertiesRoot::PropertiesRoot(UIControl *control)
{
    MakeControlPropertiesSection(control, control->GetTypeInfo(), NULL, COPY_VALUES);
    MakeBackgroundPropertiesSection(control, NULL, COPY_VALUES);
    MakeInternalControlPropertiesSection(control, NULL, COPY_VALUES);
}

PropertiesRoot::PropertiesRoot(UIControl *control, const PropertiesRoot *sourceProperties, eCopyType copyType)
{
    MakeControlPropertiesSection(control, control->GetTypeInfo(), sourceProperties, copyType);
    MakeBackgroundPropertiesSection(control, sourceProperties, copyType);
    MakeInternalControlPropertiesSection(control, sourceProperties, copyType);
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

void PropertiesRoot::Serialize(PackageSerializer *serializer) const
{
    for (const auto section : controlProperties)
        section->Serialize(serializer);

    bool hasChanges = false;
    
    for (const auto section : backgroundProperties)
    {
        if (section->HasChanges())
        {
            hasChanges = true;
            break;
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

        for (const auto section : backgroundProperties)
            section->Serialize(serializer);

        for (const auto section : internalControlProperties)
            section->Serialize(serializer);
        
        serializer->EndArray();
    }
}

String PropertiesRoot::GetName() const {
    return "ROOT";
}

BaseProperty::ePropertyType PropertiesRoot::GetType() const {
    return TYPE_HEADER;
}

void PropertiesRoot::MakeControlPropertiesSection(DAVA::UIControl *control, const DAVA::InspInfo *typeInfo, const PropertiesRoot *sourceProperties, eCopyType copyType)
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
        ControlPropertiesSection *sourceSection = sourceProperties == NULL ? NULL : sourceProperties->GetControlPropertiesSection(typeInfo->Name());
        ControlPropertiesSection *section = new ControlPropertiesSection(control, typeInfo, sourceSection, copyType);
        section->SetParent(this);
        controlProperties.push_back(section);
    }
}

void PropertiesRoot::MakeBackgroundPropertiesSection(DAVA::UIControl *control, const PropertiesRoot *sourceProperties, eCopyType copyType)
{
    for (int i = 0; i < control->GetBackgroundComponentsCount(); i++)
    {
        BackgroundPropertiesSection *sourceSection = sourceProperties == NULL ? NULL : sourceProperties->GetBackgroundPropertiesSection(i);
        BackgroundPropertiesSection *section = new BackgroundPropertiesSection(control, i, sourceSection, copyType);
        section->SetParent(this);
        backgroundProperties.push_back(section);
    }
}

void PropertiesRoot::MakeInternalControlPropertiesSection(DAVA::UIControl *control, const PropertiesRoot *sourceProperties, eCopyType copyType)
{
    for (int i = 0; i < control->GetInternalControlsCount(); i++)
    {
        InternalControlPropertiesSection *sourceSection = sourceProperties == NULL ? NULL : sourceProperties->GetInternalControlPropertiesSection(i);
        InternalControlPropertiesSection *section = new InternalControlPropertiesSection(control, i, sourceSection, copyType);
        section->SetParent(this);
        internalControlProperties.push_back(section);
    }
}
