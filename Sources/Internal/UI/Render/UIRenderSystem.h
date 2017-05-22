#pragma once

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "UI/UISystem.h"
#include "UI/UIGeometricData.h"

namespace DAVA
{
class Color;
class RenderSystem2D;
class UIControlBackground;
class UIScreenshoter;
class UIScreen;
class UIScreenTransition;

class UIRenderSystem final
: public UISystem
{
    friend class UIControlSystem;

public:
    UIRenderSystem(RenderSystem2D* renderSystem2D);
    ~UIRenderSystem() override;

    const UIGeometricData& GetBaseGeometricData() const;
    UIScreenshoter* GetScreenshoter() const;
    int32 GetUI3DViewCount() const;

    void SetClearColor(const Color& clearColor);
    void SetUseClearPass(bool useClearPass);

    void SetCurrentScreen(const RefPtr<UIScreen>& screen);
    void SetCurrentScreenTransition(const RefPtr<UIScreenTransition>& screenTransition);
    void SetPopupContainer(const RefPtr<UIControl>& popupContainer);

protected:
    void OnControlVisible(UIControl* control) override;
    void OnControlInvisible(UIControl* control) override;

    void Process(float32 elapsedTime) override;
    void Render();

private:
    void ForceRenderControl(UIControl* control);

    void RenderControlHierarhy(UIControl* control, const UIGeometricData& geometricData, const UIControlBackground* parentBackground);

    void DebugRender(UIControl* control, const UIGeometricData& geometricData, const Rect& unrotatedRect);
    void RenderDebugRect(UIControl* control, const UIGeometricData& geometricData, bool useAlpha = false);
    void RenderPivotPoint(UIControl* control, const Rect& drawRect);

    RenderSystem2D* renderSystem2D = nullptr;
    UIGeometricData baseGeometricData;

    std::unique_ptr<UIScreenshoter> screenshoter;

    RefPtr<UIScreen> currentScreen;
    RefPtr<UIControl> popupContainer;
    RefPtr<UIScreenTransition> currentScreenTransition;

    int32 ui3DViewCount = 0;
    bool needClearMainPass = true;
};
}
