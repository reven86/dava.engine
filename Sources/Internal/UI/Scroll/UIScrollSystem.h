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
    void PrepareForScreenshot(UIControl* control);

    void ScheduleScrollToControl(UIControl* control);
    void ScheduleScrollToControlWithAnimation(UIControl* control, float32 animationTime);

private:
    struct ScheduledControl
    {
        ScheduledControl(UIControl* control_, bool withAnimation_ = false, float32 animationTime_ = 0.3f)
            : withAnimation(withAnimation_)
            , animationTime(animationTime_)
        {
            control = control_;
        }

        RefPtr<UIControl> control;
        bool withAnimation = false;
        float32 animationTime = 0;
        bool processed = false;
    };

    void PrepareForScreenshotImpl(UIControl* control);
    void ScheduleScrollToControlImpl(UIControl* control, bool withAnimation, float32 animationTime);
    void ApplyScrollToScheduledControl(const ScheduledControl& c);
    Vector<UIScrollViewContainer*> scrollViewContainers;

    Vector<ScheduledControl> scheduledControls;
};
}
