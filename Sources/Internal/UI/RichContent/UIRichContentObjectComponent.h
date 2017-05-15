#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UIRichContentObjectComponent : public UIBaseComponent<UIRichContentObjectComponent>
{
    DAVA_VIRTUAL_REFLECTION(UIRichContentObjectComponent, UIBaseComponent<UIRichContentObjectComponent>);

public:
    UIRichContentObjectComponent();
    UIRichContentObjectComponent(const UIRichContentObjectComponent& src);
    UIRichContentObjectComponent& operator=(const UIRichContentObjectComponent& src) = delete;
    UIRichContentObjectComponent* Clone() const override;

    void SetPackagePath(const String& path);
    const String& GetPackagePath() const;

    void SetControlName(const String& name);
    const String& GetControlName() const;

    void SetPrototypeName(const String& name);
    const String& GetPrototypeName() const;

protected:
    ~UIRichContentObjectComponent() override;

private:
    String packagePath;
    String controlName;
    String prototypeName;
};

inline const String& UIRichContentObjectComponent::GetPackagePath() const
{
    return packagePath;
}

inline const String& UIRichContentObjectComponent::GetControlName() const
{
    return controlName;
}

inline const String& UIRichContentObjectComponent::GetPrototypeName() const
{
    return prototypeName;
}
}