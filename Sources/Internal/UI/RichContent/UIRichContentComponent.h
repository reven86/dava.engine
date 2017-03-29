#pragma once

#include "UI/Components/UIComponent.h"

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

    void ResetModify();
    bool IsModified() const;

protected:
    virtual ~UIRichContentComponent();

private:
    UIRichContentComponent& operator=(const UIRichContentComponent&) = delete;

    String text;
    String baseClasses;
    bool modified = false;

public:
    INTROSPECTION_EXTEND(UIRichContentComponent, UIComponent,
                         PROPERTY("text", "Text", GetUTF8Text, SetUTF8Text, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("baseClasses", "Base Classes", GetBaseClasses, SetBaseClasses, I_SAVE | I_VIEW | I_EDIT))
};

inline const String& DAVA::UIRichContentComponent::GetUTF8Text() const
{
    return text;
}

inline const String& DAVA::UIRichContentComponent::GetBaseClasses() const
{
    return baseClasses;
}

inline bool DAVA::UIRichContentComponent::IsModified() const
{
    return modified;
}
}
