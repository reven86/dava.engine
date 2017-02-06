#pragma once

#include <GridTest.h>

#include <DAVAEngine.h>
#include <Base/ScopedPtr.h>
#include <Scene3D/Scene.h>
#include <UI/UI3DView.h>
#include <FileSystem/FilePath.h>

namespace DAVA
{
class Engine;
class Window;
}

class ViewSceneScreen;
class PerformanceResultsScreen;

struct SceneViewerData
{
    DAVA::Engine& engine;
    DAVA::float32 screenAspect;
    DAVA::FilePath scenePath;
    DAVA::ScopedPtr<DAVA::Scene> scene;
    GridTestResult gridTestResult;
};

class SceneViewerApp final
{
public:
    SceneViewerApp(DAVA::Engine& e);

    void OnAppStarted();
    void OnWindowCreated(DAVA::Window* w);
    void OnAppFinished();

    void OnSuspend();
    void OnResume();

    void BeginFrame();
    void Draw(DAVA::Window* window);
    void EndFrame();

private:
    void CreateDocumentsFolder();

    ViewSceneScreen* viewSceneScreen = nullptr;
    PerformanceResultsScreen* performanceResultsScreen = nullptr;

    SceneViewerData data;
};
