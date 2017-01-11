#pragma once

#include "DAVAEngine.h"
#include "Database/MongodbClient.h"

namespace DAVA
{
class Engine;
class Window;
}

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
    void Draw(DAVA::Window* window);
    void EndFrame();

protected:
    void CreateDocumentsFolder();

    ViewSceneScreen* viewSceneScreen;

private:
    DAVA::Engine& engine;
    static GameCore* instance;
};
