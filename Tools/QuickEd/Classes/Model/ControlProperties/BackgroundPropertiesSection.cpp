#include "BackgroundPropertiesSection.h"

#include "IntrospectionProperty.h"
#include "PropertyVisitor.h"

#include "UI/UIControl.h"

using namespace DAVA;

BackgroundPropertiesSection::BackgroundPropertiesSection(UIControl* aControl, int aBgNum, const BackgroundPropertiesSection* sourceSection, eCloneType cloneType)
    : SectionProperty(aControl->GetBackgroundComponentName(aBgNum))
    , control(SafeRetain(aControl))
    , bg(nullptr)
    , bgNum(aBgNum)
{
    bg = SafeRetain(control->GetBackgroundComponent(bgNum));
    if (bg == nullptr && sourceSection != nullptr && sourceSection->GetBg() != nullptr)
    {
        bg = control->CreateBackgroundComponent(bgNum);
        control->SetBackgroundComponent(bgNum, bg);
    }

    if (bg)
    {
        const InspInfo* insp = bg->GetTypeInfo();

        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember* member = insp->Member(j);

            IntrospectionProperty* sourceProp = sourceSection == nullptr ? nullptr : sourceSection->FindProperty(member);
            IntrospectionProperty* prop = new IntrospectionProperty(bg, member, sourceProp, cloneType);
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

UIControlBackground* BackgroundPropertiesSection::GetBg() const
{
    return bg;
}

void BackgroundPropertiesSection::CreateControlBackground()
{
    if (!bg)
    {
        bg = control->CreateBackgroundComponent(bgNum);
        control->SetBackgroundComponent(bgNum, bg);

        const InspInfo* insp = bg->GetTypeInfo();
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember* member = insp->Member(j);
            IntrospectionProperty* prop = new IntrospectionProperty(bg, member, nullptr, CT_COPY);
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}
bool BackgroundPropertiesSection::HasChanges() const
{
    return bg && SectionProperty<IntrospectionProperty>::HasChanges();
}

void BackgroundPropertiesSection::Accept(PropertyVisitor* visitor)
{
    visitor->VisitBackgroundSection(this);
}
