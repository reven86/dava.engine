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
    struct Link final
    {
        Link(UIRichContentComponent* c);

        void AddItem(const RefPtr<UIControl>& item);
        void RemoveItems();

        UIRichContentComponent* component = nullptr;
        Vector<RefPtr<UIControl>> richItems;
    };

    void AddLink(UIRichContentComponent* component);
    void RemoveLink(UIRichContentComponent* component);

    Vector<Link> links;
    Vector<Link> appendLinks;
};
}
