#pragma once

#include "Base/BaseTypes.h"
#include "UI/UISystem.h"

namespace DAVA
{
class UIScrollViewContainer;

class UIScrollSystem : public UISystem
{
public:
    UIScrollSystem();
    ~UIScrollSystem() override;

    void RegisterControl(UIControl* control) override;
    void UnregisterControl(UIControl* control) override;
    void RegisterComponent(UIControl* control, UIComponent* component) override;
    void UnregisterComponent(UIControl* control, UIComponent* component) override;

    void Process(DAVA::float32 elapsedTime) override;

private:
    Vector<UIScrollViewContainer*> scrollViewContainers;
};
}
