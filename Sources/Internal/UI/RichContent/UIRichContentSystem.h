#pragma once

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "UI/UISystem.h"

namespace DAVA
{
class UIControl;
class UIRichContentAliasesComponent;
class UIRichContentComponent;

class UIRichContentSystem final : public UISystem
{
public:
    UIRichContentSystem() = default;
    ~UIRichContentSystem() override = default;

    void SetEditorMode(bool editorMode);
    bool IsEditorMode() const;

    void RegisterControl(UIControl* control) override;
    void UnregisterControl(UIControl* control) override;
    void RegisterComponent(UIControl* control, UIComponent* component) override;
    void UnregisterComponent(UIControl* control, UIComponent* component) override;

    void Process(float32 elapsedTime) override;

private:
    struct Link final
    {
        UIControl* control = nullptr;
        UIRichContentComponent* component = nullptr;
        Vector<UIRichContentAliasesComponent*> aliasesComponents;
        Vector<RefPtr<UIControl>> richItems;

        void AddItem(const RefPtr<UIControl>& item);
        void RemoveItems();
        void AddAliases(UIRichContentAliasesComponent* component);
        void RemoveAliases(UIRichContentAliasesComponent* component);
    };

    void AddLink(UIRichContentComponent* component);
    void RemoveLink(UIRichContentComponent* component);
    void AddAliases(UIControl* control, UIRichContentAliasesComponent* component);
    void RemoveAliases(UIControl* control, UIRichContentAliasesComponent* component);

    Vector<Link> links;
    Vector<Link> appendLinks;
    bool isEditorMode = false;

    friend class XMLRichContentBuilder;
};

inline bool UIRichContentSystem::IsEditorMode() const
{
    return isEditorMode;
}
}
