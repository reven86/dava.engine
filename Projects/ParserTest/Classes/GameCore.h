#ifndef __GAMECORE_H__
#define __GAMECORE_H__

#include "DAVAEngine.h"
#include "Database/MongodbClient.h"

#include "Render/RHI/Common/rhi_Private.h"
#include "Render/RHI/rhi_Public.h"

//#include "SceneRenderTestV3.h"

#include <memory>

using namespace DAVA;

class GameCore : public ApplicationCore
{
protected:
    virtual ~GameCore();

public:
    GameCore();

    static GameCore* Instance()
    {
        return (GameCore*)DAVA::Core::GetApplicationCore();
    };

    virtual void OnAppStarted();
    virtual void OnAppFinished();

    virtual void OnSuspend();
    virtual void OnResume();

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    virtual void OnBackground();
    virtual void OnForeground();
    virtual void OnDeviceLocked();
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

    virtual void Update(float32 timeElapsed);

    virtual void BeginFrame();
    virtual void Draw();
    virtual void EndFrame();

    void OnKeyUp(UIEvent* evt);

protected:
    void _test_parser();
};



#endif // __GAMECORE_H__