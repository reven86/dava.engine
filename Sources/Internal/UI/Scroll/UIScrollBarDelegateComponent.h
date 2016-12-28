#pragma once

#include "Base/BaseTypes.h"

#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UIScrollBarDelegateComponent : public UIBaseComponent<UIComponent::SCROLL_BAR_DELEGATE_COMPONENT>
{
public:
    UIScrollBarDelegateComponent();
    UIScrollBarDelegateComponent(const UIScrollBarDelegateComponent& src);

protected:
    virtual ~UIScrollBarDelegateComponent();

    UIScrollBarDelegateComponent& operator=(const UIScrollBarDelegateComponent&) = delete;

public:
    UIScrollBarDelegateComponent* Clone() const override;

    const String& GetPathToDelegate() const;
    void SetPathToDelegate(const String& path);

    bool IsPathToDelegateDirty() const;
    void ResetPathToDelegateDirty();

private:
    String pathToDelegate;
    bool pathToDelegateDirty = true;

public:
    INTROSPECTION_EXTEND(UIScrollBarDelegateComponent, UIComponent,
                         PROPERTY("delegate", "Delegate", GetPathToDelegate, SetPathToDelegate, I_SAVE | I_VIEW | I_EDIT)
                         );
};
}
