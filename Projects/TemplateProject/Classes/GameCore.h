#ifndef __GAMECORE_H__
#define __GAMECORE_H__

#include "DAVAEngine.h"

class TestScreen;

class GameCore : public DAVA::ApplicationCore
{
protected:
    virtual ~GameCore();

public:
    GameCore();

    virtual void OnAppStarted();
    virtual void OnAppFinished();

    virtual void OnSuspend();
    virtual void OnResume();
    virtual void OnBackground();
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    virtual void OnDeviceLocked();
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

    virtual void BeginFrame();
    virtual void Update(DAVA::float32 update);
    virtual void Draw();

private:
    DAVA::Cursor* cursor;
    TestScreen* testScreen;
};



#endif // __GAMECORE_H__