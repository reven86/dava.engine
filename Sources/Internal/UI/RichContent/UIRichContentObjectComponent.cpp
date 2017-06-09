#include "UIRichContentObjectComponent.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIRichContentObjectComponent)
{
    ReflectionRegistrator<UIRichContentObjectComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIRichContentObjectComponent* o) { o->Release(); })
    .Field("packagePath", &UIRichContentObjectComponent::GetPackagePath, &UIRichContentObjectComponent::SetPackagePath)
    .Field("controlName", &UIRichContentObjectComponent::GetControlName, &UIRichContentObjectComponent::SetControlName)
    .Field("prototypeName", &UIRichContentObjectComponent::GetPrototypeName, &UIRichContentObjectComponent::SetPrototypeName)
    .End();
}

UIRichContentObjectComponent::UIRichContentObjectComponent() = default;
UIRichContentObjectComponent::~UIRichContentObjectComponent() = default;

UIRichContentObjectComponent::UIRichContentObjectComponent(const UIRichContentObjectComponent& src)
    : packagePath(src.packagePath)
    , controlName(src.controlName)
    , prototypeName(src.prototypeName)
{
}

UIRichContentObjectComponent* UIRichContentObjectComponent::Clone() const
{
    return new UIRichContentObjectComponent(*this);
}

void UIRichContentObjectComponent::SetPackagePath(const String& path)
{
    packagePath = path;
}

void UIRichContentObjectComponent::SetControlName(const String& name)
{
    controlName = name;
}

void UIRichContentObjectComponent::SetPrototypeName(const String& name)
{
    prototypeName = name;
}
}