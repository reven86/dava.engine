#include "UIScrollBarDelegateComponent.h"

namespace DAVA
{
UIScrollBarDelegateComponent::UIScrollBarDelegateComponent()
{
}

UIScrollBarDelegateComponent::UIScrollBarDelegateComponent(const UIScrollBarDelegateComponent& src)
    : pathToDelegate(src.pathToDelegate)
{
}

UIScrollBarDelegateComponent::~UIScrollBarDelegateComponent()
{
}

UIScrollBarDelegateComponent* UIScrollBarDelegateComponent::Clone() const
{
    return new UIScrollBarDelegateComponent(*this);
}

const String& UIScrollBarDelegateComponent::GetPathToDelegate() const
{
    return pathToDelegate;
}

void UIScrollBarDelegateComponent::SetPathToDelegate(const String& path)
{
    pathToDelegate = path;
    pathToDelegateDirty = true;
}

bool UIScrollBarDelegateComponent::IsPathToDelegateDirty() const
{
    return pathToDelegateDirty;
}

void UIScrollBarDelegateComponent::ResetPathToDelegateDirty()
{
    pathToDelegateDirty = false;
}
}
