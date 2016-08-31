#ifndef __DAVAENGINE_UI_SCREEN_TRANSITION_H__
#define __DAVAENGINE_UI_SCREEN_TRANSITION_H__

#include "Base/BaseTypes.h"
#include "UI/UIScreen.h"

namespace DAVA
{
class UIScreenTransition : public UIScreen, public TrackedObject
{
public:
    UIScreenTransition();

protected:
    ~UIScreenTransition() override;

public:
    void Draw(const UIGeometricData& geometricData) override;

    void OnVisible() override;
    void OnInvisible() override;

    virtual void SetSourceScreen(UIControl* prevScreen, bool updateScreen = true);
    virtual void SetDestinationScreen(UIControl* nextScreen, bool updateScreen = true);
    virtual void StartTransition();
    virtual void EndTransition();

    virtual void SetDuration(float32 timeInSeconds);

    bool IsComplete() const;

protected:
    void Update(float32 timeElapsed);
    void CreateRenderTargets();
    void ReleaseRenderTargets();
    Sprite* renderTargetPrevScreen = nullptr;
    Sprite* renderTargetNextScreen = nullptr;

    Interpolation::Func interpolationFunc;
    float32 currentTime = 0.0f;
    float32 duration = 0.7f;
    float32 normalizedTime = 0.0f;
    float32 scale = 1.0f;
    bool complete = false;
};
};



#endif // __DAVAENGINE_UI_SCREEN_TRANSITION_H__