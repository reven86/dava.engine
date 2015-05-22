#include "BackgroundPropertiesSection.h"

#include "IntrospectionProperty.h"

#include "UI/UIControl.h"
#include "Model/PackageSerializer.h"

using namespace DAVA;

BackgroundPropertiesSection::BackgroundPropertiesSection(UIControl *aControl, int aBgNum, const BackgroundPropertiesSection *sourceSection, eCloneType cloneType)
    : SectionProperty(aControl->GetBackgroundComponentName(aBgNum))
    , control(SafeRetain(aControl))
    , bg(nullptr)
    , bgNum(aBgNum)
{
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
            ValueProperty *prop = new IntrospectionProperty(bg, member, dynamic_cast<IntrospectionProperty*>(sourceProp), cloneType);
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
            IntrospectionProperty *prop = new IntrospectionProperty(bg, member, nullptr, CT_COPY);
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
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
