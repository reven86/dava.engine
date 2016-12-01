#pragma once

#include "UI/UISystem.h"

namespace DAVA
{
class UIScrollBarDelegateComponent;

class UIScrollBarLinkSystem : public UISystem
{
public:
    UIScrollBarLinkSystem();
    ~UIScrollBarLinkSystem() override;

    void RegisterControl(UIControl* control) override;
    void UnregisterControl(UIControl* control) override;
    void RegisterComponent(UIControl* control, UIComponent* component) override;
    void UnregisterComponent(UIControl* control, UIComponent* component) override;

    void Process(DAVA::float32 elapsedTime) override;

    void SetRestoreLinks(bool restoring);

private:
    struct Link
    {
        enum Type
        {
            SCROLL_BAR_LINKED = 0x01,
            DELEGATE_LINKED = 0x02
        };
        UIScrollBarDelegateComponent* component = nullptr;
        UIControl* linkedControl = nullptr;
        int32 connectionType = 0;

        Link(UIScrollBarDelegateComponent* component_)
            : component(component_)
            , connectionType(SCROLL_BAR_LINKED)
        {
        }
    };

    void RegisterScrollBarDelegateComponent(UIScrollBarDelegateComponent* component);
    void UnregisterScrollBarDelegateComponent(UIScrollBarDelegateComponent* component);
    void SetupLink(Link* link, UIControl* control);
    void BreakLink(Link* link);

    bool TryToRestoreLink(UIScrollBarDelegateComponent* component, UIControl* linkedControl);
    bool TryToBreakLink(UIScrollBarDelegateComponent* component, UIControl* linkedControl);

    Vector<Link> links;

    bool restoreLinks = true;
};
}
