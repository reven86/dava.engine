#pragma once

#include "Base/BaseTypes.h"
#include "UI/UISystem.h"

namespace DAVA
{
class UIControl;
class UIComponent;
class UICustomUpdateDeltaComponent;

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
    UICustomUpdateDeltaComponent* FindParentComponent(UIControl* ctrl);

    struct UpdateBind
    {
        UpdateBind(UIComponent* uc, UICustomUpdateDeltaComponent* cdc)
            : updateComponent(uc)
            , customDeltaComponent(cdc)
        {
        }

        UIComponent* updateComponent = nullptr;
        UICustomUpdateDeltaComponent* customDeltaComponent = nullptr;
    };

    List<UpdateBind> binds;
};
}