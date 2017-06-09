#pragma once

#include "Functional/Signal.h"
#include "Reflection/Reflection.h"
#include "UI/Components/UIComponent.h"
#include "UI/RichContent/UIRichAliasMap.h"

namespace DAVA
{
class UIControl;

/**
    Describe rich content inside control.
    Format of input text source it xml (or xhtml).

    For default all text content in any tags will be added as UIStaticText controls.
    Also all tags support attribute `class` witch set specified class to created
    UIStaticText controls.

    Special rules available for next tags:
    - <p> - paragraph.
        Text content inside this tag will be wrap by new lines.
    - <ul> and <li> - unordered list and list item.
        Text content inside these tags will be wrap by new lines and before all
        lines in `li` tag will be added bullet symbol.
    - <img src="..."> - image.
        Insert UIControl with UIControlBackground component and set intro it
        sprite by path specified in `src` attribute.
    - <object path="..." control="..." prototype="..." name="..."> - custom control.
        Insert clone of specified control by root control name or prototype name
        witch contain in package by specialized path. Also set name to this clone
        from name attribute. Attribute `path` and one of attributes `control` or
        `prototype` are required. Attribute `name` is optional.

    RichContent compoent also support tag aliases. Any alias contains alias
    name, original tag and original attributes. After declaration of alias you
    can use it like an any tag in the content source. But all attributes witch
    will be added to alias in content source will be ignored and replaced by
    attributes from alias data.

    For example:
    <code>
        Defined aliases:
        h1 = <p class="header" />

        Source:
        <h1 class="ordinary_text">Header</h1>
    </code>
    In this case tag `h1` with its attributes will be raplaced by tag `p` with
    attributes `class="header"`.

*/
class UIRichContentComponent : public UIBaseComponent<UIComponent::RICH_CONTENT_COMPONENT>
{
    DAVA_VIRTUAL_REFLECTION(UIRichContentComponent, UIBaseComponent<UIComponent::RICH_CONTENT_COMPONENT>);

public:
    /** Default constructor. */
    UIRichContentComponent() = default;
    /** Copy constructor. */
    UIRichContentComponent(const UIRichContentComponent& src);
    /** Removed operator overloading. */
    UIRichContentComponent& operator=(const UIRichContentComponent&) = delete;

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
    /** Return aliases for tags. */
    const UIRichAliasMap& GetAliases() const;

    /** Set aliases for tags from specified string. */
    void SetAliasesFromString(const String& aliases);
    /** Return aliases for tags as string. */
    const String& GetAliasesAsString();

    /** Set modification flag. */
    void SetModified(bool modified);
    /** Return value of modification flag. */
    bool IsModified() const;

    /** Emit signal on create control by `<object />` tag */
    Signal<UIControl* /* createdControl */> onCreateObject;

protected:
    ~UIRichContentComponent() override = default;

private:
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
