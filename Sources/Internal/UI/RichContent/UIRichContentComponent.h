#pragma once

#include "UI/Components/UIComponent.h"
#include "UI/RichContent/UIRichAliasMap.h"

namespace DAVA
{
class UIControl;

class UIRichContentComponent : public UIBaseComponent<UIComponent::RICH_CONTENT_COMPONENT>
{
public:
    UIRichContentComponent();
    UIRichContentComponent(const UIRichContentComponent& src);

    UIRichContentComponent* Clone() const override;

    void SetUTF8Text(const String& text);
    const String& GetUTF8Text() const;

    void SetBaseClasses(const String& classes);
    const String& GetBaseClasses() const;

    void SetAliases(const UIRichAliasMap& aliases);
    const UIRichAliasMap& GetAliases() const;

    void ResetModify();
    bool IsModified() const;

protected:
    virtual ~UIRichContentComponent();

private:
    UIRichContentComponent& operator=(const UIRichContentComponent&) = delete;

    void SetAliasesFromString(const String& aliases);
    String GetAliasesAsString() const;

    String text;
    String baseClasses;
    UIRichAliasMap aliases;
    bool modified = false;

public:
    INTROSPECTION_EXTEND(UIRichContentComponent, UIComponent,
                         PROPERTY("text", "Text", GetUTF8Text, SetUTF8Text, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("baseClasses", "Base Classes", GetBaseClasses, SetBaseClasses, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("aliases", "Aliases", GetAliasesAsString, SetAliasesFromString, I_SAVE | I_VIEW | I_EDIT))
};

inline const String& UIRichContentComponent::GetUTF8Text() const
{
    return text;
}

inline const String& UIRichContentComponent::GetBaseClasses() const
{
    return baseClasses;
}

inline bool UIRichContentComponent::IsModified() const
{
    return modified;
}

inline const UIRichAliasMap& UIRichContentComponent::GetAliases() const
{
    return aliases;
}
}
