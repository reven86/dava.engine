#include "UIRichContentComponent.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIRichContentComponent)
{
    ReflectionRegistrator<UIRichContentComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIRichContentComponent* o) { o->Release(); })
    .Field("text", &UIRichContentComponent::GetText, &UIRichContentComponent::SetText)
    .Field("baseClasses", &UIRichContentComponent::GetBaseClasses, &UIRichContentComponent::SetBaseClasses)
    .Field("classesInheritance", &UIRichContentComponent::GetClassesInheritance, &UIRichContentComponent::SetClassesInheritance)
    .End();
}

UIRichContentComponent::UIRichContentComponent(const UIRichContentComponent& src)
    : UIBaseComponent(src)
    , text(src.text)
    , baseClasses(src.baseClasses)
    , classesInheritance(src.classesInheritance)
    , modified(true)
{
}

UIRichContentComponent* UIRichContentComponent::Clone() const
{
    return new UIRichContentComponent(*this);
}

void UIRichContentComponent::SetText(const String& _text)
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

void UIRichContentComponent::SetClassesInheritance(bool inheritance)
{
    if (classesInheritance != inheritance)
    {
        classesInheritance = inheritance;
        modified = true;
    }
}

void UIRichContentComponent::SetModified(bool _modified)
{
    modified = _modified;
}

}
