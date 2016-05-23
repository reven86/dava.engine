#ifndef __GAMECORE_H__
#define __GAMECORE_H__

#include "DAVAEngine.h"
#include "Database/MongodbClient.h"

using namespace DAVA;

class SelectSceneScreen;
class ViewSceneScreen;
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

    virtual void BeginFrame();
    virtual void EndFrame();

    void SetScenePath(const FilePath& path)
    {
        scenePath = path;
    };
    const FilePath& GetScenePath() const
    {
        return scenePath;
    };

protected:
    void CreateDocumentsFolder();

    SelectSceneScreen* selectSceneScreen;
    ViewSceneScreen* viewSceneScreen;

    rhi::HPerfQuerySet perfQuerySet;
    bool perfQuerySetFired;

    FilePath scenePath;
};



#endif // __GAMECORE_H__
