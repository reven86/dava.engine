#pragma once

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "UI/UISystem.h"

namespace DAVA
{
class UIControl;
class UIRichContentAliasesComponent;
class UIRichContentComponent;
struct RichLink;

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
    void AddLink(UIRichContentComponent* component);
    void RemoveLink(UIRichContentComponent* component);
    void AddAliases(UIControl* control, UIRichContentAliasesComponent* component);
    void RemoveAliases(UIControl* control, UIRichContentAliasesComponent* component);

    Vector<std::shared_ptr<RichLink>> links;
    Vector<std::shared_ptr<RichLink>> appendLinks;
    bool isEditorMode = false;
};

inline bool UIRichContentSystem::IsEditorMode() const
{
    return isEditorMode;
}
}
