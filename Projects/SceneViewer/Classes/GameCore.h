#pragma once

#include "DAVAEngine.h"
#include "Database/MongodbClient.h"

namespace DAVA
{
class Engine;
class Window;
}

class SelectSceneScreen;
class ViewSceneScreen;
class GameCore
{
public:
    GameCore(DAVA::Engine& e);

    static GameCore* Instance()
    {
        return instance;
    };

    void OnAppStarted();
    void OnWindowCreated(DAVA::Window* w);
    void OnAppFinished();

    void OnSuspend();
    void OnResume();

    void BeginFrame();
    void Draw();
    void EndFrame();

    void SetScenePath(const DAVA::FilePath& path)
    {
        scenePath = path;
    };

    const DAVA::FilePath& GetScenePath() const
    {
        return scenePath;
    };

protected:
    void CreateDocumentsFolder();

    SelectSceneScreen* selectSceneScreen;
    ViewSceneScreen* viewSceneScreen;

    rhi::HPerfQuerySet perfQuerySet;
    bool perfQuerySetFired;

    DAVA::FilePath scenePath;

private:
    DAVA::Engine& engine;

    static GameCore* instance;
};
