#pragma once

#include <Base/ScopedPtr.h>
#include <CommandLine/ProgramOptions.h>
#include <FileSystem/FilePath.h>

namespace DAVA
{
class Engine;
class Window;
}

class UIViewScreen;
class UIViewerApp final
{
public:
    UIViewerApp(DAVA::Engine& e, const DAVA::Vector<DAVA::String>& cmdLine);

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

    UIViewScreen* uiViewScreen = nullptr;
    DAVA::Engine& engine;

    DAVA::ProgramOptions options;
    bool optionsAreParsed = true;
};
