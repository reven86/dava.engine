#include "BackgroundPropertiesSection.h"

#include "ValueProperty.h"

#include "UI/UIControl.h"
#include "../PackageSerializer.h"

using namespace DAVA;

BackgroundPropertiesSection::BackgroundPropertiesSection(UIControl *control, int bgNum, const BackgroundPropertiesSection *sourceSection, eCopyType copyType) : control(NULL), bg(NULL), bgNum(bgNum)
{
    this->control = SafeRetain(control);
    
    bg = SafeRetain(control->GetBackgroundComponent(bgNum));
    if (bg == NULL && sourceSection != NULL && sourceSection->GetBg() != NULL)
    {
        bg = control->CreateBackgroundComponent(bgNum);
        control->SetBackgroundComponent(bgNum, bg);
    }
    
    if (bg)
    {
        const InspInfo *insp = bg->GetTypeInfo();
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember *member = insp->Member(j);
            
            ValueProperty *sourceProp = sourceSection == NULL ? NULL : sourceSection->FindProperty(member);
            ValueProperty *prop = new ValueProperty(bg, member, sourceProp, copyType);
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

BackgroundPropertiesSection::~BackgroundPropertiesSection()
{
    SafeRelease(control);
    SafeRelease(bg);
}

UIControlBackground *BackgroundPropertiesSection::GetBg() const
{
    return bg;
}

void BackgroundPropertiesSection::CreateControlBackground()
{
    if (!bg)
    {
        bg = control->CreateBackgroundComponent(bgNum);
        control->SetBackgroundComponent(bgNum, bg);
        
        const InspInfo *insp = bg->GetTypeInfo();
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember *member = insp->Member(j);
            ValueProperty *prop = new ValueProperty(bg, member, NULL, COPY_VALUES);
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

DAVA::String BackgroundPropertiesSection::GetName() const
{
    return control->GetBackgroundComponentName(bgNum);
}

bool BackgroundPropertiesSection::HasChanges() const
{
    return bg && SectionProperty::HasChanges();
}

void BackgroundPropertiesSection::Serialize(PackageSerializer *serializer) const
{
    if (HasChanges())
    {
        serializer->BeginMap(GetName());
        
        for (const auto child : children)
            child->Serialize(serializer);

        serializer->EndMap();
    }
}
