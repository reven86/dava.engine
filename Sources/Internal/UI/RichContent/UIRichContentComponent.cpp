#include "UIRichContentComponent.h"

namespace DAVA
{
UIRichContentComponent::UIRichContentComponent()
{
}

UIRichContentComponent::UIRichContentComponent(const UIRichContentComponent& src)
    : text(src.text)
    , baseClasses(src.baseClasses)
    , modified(true)
{
}

UIRichContentComponent::~UIRichContentComponent()
{
}

UIRichContentComponent* UIRichContentComponent::Clone() const
{
    return new UIRichContentComponent(*this);
}

void UIRichContentComponent::SetUTF8Text(const String& _text)
{
    if (text != _text)
    {
        text = _text;
        modified = true;
    }
}

void UIRichContentComponent::SetBaseClasses(const String& classes)
{
    if (baseClasses != classes)
    {
        baseClasses = classes;
        modified = true;
    }
}

void UIRichContentComponent::ResetModify()
{
    modified = false;
}
}
