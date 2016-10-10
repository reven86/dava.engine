#pragma once

#include "REBaseApplication.h"

class QGuiApplication;
class QOffscreenSurface;
class QOpenGLContext;

class WinConsoleIOLocker;
class CommandLineManager;
class REConsoleApplication : public REBaseApplication
{
public:
    REConsoleApplication(CommandLineManager& cmdLineManager);
    ~REConsoleApplication() = default; // on MacOS destructor will never be called. All clean up should be done in "OnLoopStopped"

    void OnLoopStarted() override;
    void OnLoopStopped() override;
    void OnUpdate(DAVA::float32 delta) override;

protected:
    DAVA::eEngineRunMode GetEngineMode() const override;

private:
#if defined(__DAVAENGINE_WIN32__)
    WinConsoleIOLocker* locker;
#endif //platforms

    CommandLineManager& cmdLineManager;
    QGuiApplication* application = nullptr;
    QOffscreenSurface* surface = nullptr;
    QOpenGLContext* context = nullptr;
    int argc = 0;
    DAVA::Vector<char*> argv;
};
