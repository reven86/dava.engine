#include "UILayoutSourceRectComponent.h"

#include "UI/UIControl.h"
#include "Math/Vector.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UILayoutSourceRectComponent)
{
    ReflectionRegistrator<UILayoutSourceRectComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UILayoutSourceRectComponent* o) { o->Release(); })
    .End();
}

UILayoutSourceRectComponent::UILayoutSourceRectComponent()
{
}

UILayoutSourceRectComponent::UILayoutSourceRectComponent(const UILayoutSourceRectComponent& src)
    : postion(src.postion)
    , size(src.size)
{
}

UILayoutSourceRectComponent::~UILayoutSourceRectComponent()
{
}

UILayoutSourceRectComponent* UILayoutSourceRectComponent::Clone() const
{
    return new UILayoutSourceRectComponent(*this);
}

const Vector2& UILayoutSourceRectComponent::GetPosition() const
{
    return postion;
}

void UILayoutSourceRectComponent::SetPosition(const Vector2& position_)
{
    postion = position_;
    SetLayoutDirty();
}

const Vector2& UILayoutSourceRectComponent::GetSize() const
{
    return size;
}

void UILayoutSourceRectComponent::SetSize(const Vector2& size_)
{
    size = size_;
    SetLayoutDirty();
}

void UILayoutSourceRectComponent::SetLayoutDirty()
{
    if (GetControl() != nullptr)
    {
        GetControl()->SetLayoutDirty();
    }
}
}
