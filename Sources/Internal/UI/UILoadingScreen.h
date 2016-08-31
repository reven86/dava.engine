#ifndef __DAVAENGINE_UI_LOADING_SCREEN_H__
#define __DAVAENGINE_UI_LOADING_SCREEN_H__

#include "Base/BaseTypes.h"
#include "UI/UIScreen.h"

namespace DAVA
{
class UILoadingScreen : public UIScreen, public TrackedObject
{
public:
    UILoadingScreen() = default;
    ~UILoadingScreen() override;

    virtual void SetScreenToLoad(int32 screenId);

    void OnActive() override;
    void OnInactive() override;

protected:
    void ThreadMessage(BaseObject* obj, void* userData, void* callerData);
    RefPtr<Thread> thread;

private:
    void Update(float32 timeElapsed);

    int32 nextScreenId = -1;
};
};



#endif // __DAVAENGINE_UI_LOADING_SCREEN_H__