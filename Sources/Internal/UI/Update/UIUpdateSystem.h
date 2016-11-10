#pragma once

#include "Base/BaseTypes.h"
#include "UI/UISystem.h"

namespace DAVA
{
class UIControl;
class UIComponent;
class UICustomUpdateDeltaComponent;

/**
Send update events for all visible UIControl with UIUpdateCompoent component.
Temporary system for backward compatibility with existing code.
**WILL BE REMOVED** after refactoring all `UIControl::Update` logic.
*/
class UIUpdateSystem : public UISystem
{
public:
    ~UIUpdateSystem() override = default;

    void RegisterControl(UIControl* control) override;
    void UnregisterControl(UIControl* control) override;
    void RegisterComponent(UIControl* control, UIComponent* component) override;
    void UnregisterComponent(UIControl* control, UIComponent* component) override;

    void OnControlVisible(UIControl* control) override;
    void OnControlInvisible(UIControl* control) override;

    void Process(float32 elapsedTime) override;

private:
    struct UpdateBind
    {
        UpdateBind(UIComponent* uc, UICustomUpdateDeltaComponent* cdc);
        const UIComponent* updateComponent = nullptr;
        UICustomUpdateDeltaComponent* customDeltaComponent = nullptr;
    };

    List<UpdateBind> binds;
};
}