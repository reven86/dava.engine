#pragma once

#include "Base/BaseTypes.h"
#include "UI/UISystem.h"

namespace DAVA
{
class UIControl;
class UIComponent;
class UIUpdateComponent;

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
    List<UIUpdateComponent*> components;
};
}