#include "UI/Layouts/UIFlowLayoutHintComponent.h"
#include "Reflection/ReflectionRegistrator.h"

#include "UI/UIControl.h"
#include "Math/Vector.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIFlowLayoutHintComponent)
{
    ReflectionRegistrator<UIFlowLayoutHintComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIFlowLayoutHintComponent* o) { o->Release(); })
    .Field("newLineBeforeThis", &UIFlowLayoutHintComponent::IsNewLineBeforeThis, &UIFlowLayoutHintComponent::SetNewLineBeforeThis)
    .Field("newLineAfterThis", &UIFlowLayoutHintComponent::IsNewLineAfterThis, &UIFlowLayoutHintComponent::SetNewLineAfterThis)
    .Field("contentDirection", &UIFlowLayoutHintComponent::GetContentDirection, &UIFlowLayoutHintComponent::SetContentDirection)[M::EnumT<BiDiHelper::Direction>()]
    .End();
}

UIFlowLayoutHintComponent::UIFlowLayoutHintComponent()
{
}

UIFlowLayoutHintComponent::UIFlowLayoutHintComponent(const UIFlowLayoutHintComponent& src)
    : flags(src.flags)
{
}

UIFlowLayoutHintComponent::~UIFlowLayoutHintComponent()
{
}

UIFlowLayoutHintComponent* UIFlowLayoutHintComponent::Clone() const
{
    return new UIFlowLayoutHintComponent(*this);
}

bool UIFlowLayoutHintComponent::IsNewLineBeforeThis() const
{
    return flags.test(FLAG_NEW_LINE_BEFORE_THIS);
}

void UIFlowLayoutHintComponent::SetNewLineBeforeThis(bool flag)
{
    flags.set(FLAG_NEW_LINE_BEFORE_THIS, flag);
    SetLayoutDirty();
}

bool UIFlowLayoutHintComponent::IsNewLineAfterThis() const
{
    return flags.test(FLAG_NEW_LINE_AFTER_THIS);
}

void UIFlowLayoutHintComponent::SetNewLineAfterThis(bool flag)
{
    flags.set(FLAG_NEW_LINE_AFTER_THIS, flag);
    SetLayoutDirty();
}

BiDiHelper::Direction UIFlowLayoutHintComponent::GetContentDirection() const
{
    return contentDirection;
}

void UIFlowLayoutHintComponent::SetContentDirection(BiDiHelper::Direction direction)
{
    contentDirection = direction;
    SetLayoutDirty();
}

void UIFlowLayoutHintComponent::SetLayoutDirty()
{
    if (GetControl() != nullptr)
    {
        GetControl()->SetLayoutDirty();
    }
}
}
