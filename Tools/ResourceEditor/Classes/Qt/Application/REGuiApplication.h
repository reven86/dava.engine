#pragma once

#include "REBaseApplication.h"

class QtMainWindow;
class TextureCache;
class ResourceEditorLauncher;

class REGuiApplication : public REBaseApplication
{
public:
    REGuiApplication();

    void OnLoopStarted() override;
    void OnLoopStopped() override;
    void OnWindowCreated(DAVA::Window* w) override;

protected:
    DAVA::eEngineRunMode GetEngineMode() override;

private:
    void UnpackHelpDoc();
    void FixOSXFonts();

private:
    ResourceEditorLauncher* launcher = nullptr;
    TextureCache* textureCache = nullptr;
    QtMainWindow* mainWindow = nullptr;
};