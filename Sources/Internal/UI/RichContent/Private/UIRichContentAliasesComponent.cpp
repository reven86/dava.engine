#include "UI/RichContent/UIRichContentAliasesComponent.h"

#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIRichContentAliasesComponent)
{
    ReflectionRegistrator<UIRichContentAliasesComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIRichContentAliasesComponent* o) { o->Release(); })
    .Field("aliases", &UIRichContentAliasesComponent::GetAliasesAsString, &UIRichContentAliasesComponent::SetAliasesFromString)
    .End();
}

UIRichContentAliasesComponent::UIRichContentAliasesComponent(const UIRichContentAliasesComponent& src)
    : UIBaseComponent(src)
    , aliases(src.aliases)
    , modified(true)
{
}

UIRichContentAliasesComponent* UIRichContentAliasesComponent::Clone() const
{
    return new UIRichContentAliasesComponent(*this);
}

void UIRichContentAliasesComponent::SetAliases(const UIRichAliasMap& _aliases)
{
    if (aliases != _aliases)
    {
        aliases = _aliases;
        modified = true;
    }
}

void UIRichContentAliasesComponent::SetModified(bool _modified)
{
    modified = _modified;
}

void UIRichContentAliasesComponent::SetAliasesFromString(const String& _aliases)
{
    aliases.FromString(_aliases);
    modified = true;
}

const String& UIRichContentAliasesComponent::GetAliasesAsString()
{
    return aliases.AsString();
}
}
