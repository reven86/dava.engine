#pragma once

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "UI/UISystem.h"

namespace DAVA
{
class UIControl;
class UIRichContentComponent;

class UIRichContentSystem final : public UISystem
{
public:
    UIRichContentSystem() = default;
    ~UIRichContentSystem() override = default;

    void RegisterControl(UIControl* control) override;
    void UnregisterControl(UIControl* control) override;
    void RegisterComponent(UIControl* control, UIComponent* component) override;
    void UnregisterComponent(UIControl* control, UIComponent* component) override;

    void Process(float32 elapsedTime) override;

private:
    struct Link
    {
        Link(UIRichContentComponent* c)
            : component(c)
        {
        }
        UIRichContentComponent* component = nullptr;
    };

    void AddLink(UIRichContentComponent* component);
    void RemoveLink(UIRichContentComponent* component);
    void RemoveRichContentItems(UIControl* root);

    Vector<Link> links;
};
}
