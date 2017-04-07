#pragma once

#include "Reflection/Reflection.h"
#include "UI/Components/UIComponent.h"
#include "UI/RichContent/UIRichAliasMap.h"

namespace DAVA
{
class UIControl;

/** Describe rich content inside control. */
class UIRichContentComponent : public UIBaseComponent<UIComponent::RICH_CONTENT_COMPONENT>
{
    DAVA_VIRTUAL_REFLECTION(UIRichContentComponent, UIBaseComponent<UIComponent::RICH_CONTENT_COMPONENT>);

public:
    /** Defaul contructor. */
    UIRichContentComponent() = default;
    /** Copy contructor. */
    UIRichContentComponent(const UIRichContentComponent& src);

    UIRichContentComponent* Clone() const override;

    /** Set rich content text. */
    void SetText(const String& text);
    /** Return rich content text. */
    const String& GetText() const;

    /** Set top level classes for rich content. */
    void SetBaseClasses(const String& classes);
    /** Return top level classes for rich content. */
    const String& GetBaseClasses() const;

    /** Set aliases for tags with attributes. */
    void SetAliases(const UIRichAliasMap& aliases);
    /** Return aliases tof tags. */
    const UIRichAliasMap& GetAliases() const;

    /** Set aliases for tags from specified string. */
    void SetAliasesFromString(const String& aliases);
    /** Return aliases for tags as string. */
    String GetAliasesAsString() const;

    /** Set modification flag. */
    void SetModified(bool modified);
    /** Return value of modification flag. */
    bool IsModified() const;

protected:
    ~UIRichContentComponent() override = default;

private:
    UIRichContentComponent& operator=(const UIRichContentComponent&) = delete;

    String text;
    String baseClasses;
    UIRichAliasMap aliases;
    bool modified = false;
};

inline const String& UIRichContentComponent::GetText() const
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
