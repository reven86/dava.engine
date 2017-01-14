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
        const ReflectedType *reflectedType = bg->GetReflectedType();

        for (const std::unique_ptr<ReflectedStructure::Field> &field : reflectedType->GetStrucutre()->fields)
        {
            IntrospectionProperty* sourceProp = sourceSection == nullptr ? nullptr : sourceSection->FindProperty(field.get());
            IntrospectionProperty* prop = new IntrospectionProperty(bg, field.get(), sourceProp, cloneType);
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

        const ReflectedType *reflectedType = bg->GetReflectedType();
        for (const std::unique_ptr<ReflectedStructure::Field> &field : reflectedType->GetStrucutre()->fields)
        {
            IntrospectionProperty* prop = new IntrospectionProperty(bg, field.get(), nullptr, CT_COPY);
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
