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
        Reflection bgRef = Reflection::Create(&bg);
        Vector<Reflection::Field> fields = bgRef.GetFields();

        for (const Reflection::Field &field : fields)
        {
            String name = field.key.Get<String>();
            IntrospectionProperty* sourceProp = sourceSection == nullptr ? nullptr : sourceSection->FindChildPropertyByName(name);
            IntrospectionProperty* prop = new IntrospectionProperty(bg, name, field.ref, sourceProp, cloneType);
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

        Reflection bgRef = Reflection::Create(&bg);
        Vector<Reflection::Field> fields = bgRef.GetFields();
        for (const Reflection::Field &field : fields)
        {
            String name = field.key.Get<String>();
            IntrospectionProperty* prop = new IntrospectionProperty(bg, name, field.ref, nullptr, CT_COPY);
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
