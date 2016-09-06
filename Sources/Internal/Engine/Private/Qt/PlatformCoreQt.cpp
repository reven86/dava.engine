#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Qt/PlatformCoreQt.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Public/Qt/NativeServiceQt.h"
#include "Engine/Public/Qt/RenderWidget.h"
#include "Engine/Public/Window.h"

#include <QTimer>
#include <QApplication>

namespace DAVA
{
namespace Private
{
PlatformCore::PlatformCore(EngineBackend* engineBackend_)
    : engineBackend(engineBackend_)
    , nativeService(new NativeService(this))
{
}

PlatformCore::~PlatformCore() = default;

void PlatformCore::Init()
{
}

void PlatformCore::Run()
{
    Vector<char*> qtCommandLine = engineBackend->GetCommandLineAsArgv();
    int qtArgc = static_cast<int>(qtCommandLine.size());

    QApplication app(qtArgc, qtCommandLine.data());

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]()
                     {
                         DVASSERT(windowBackend != nullptr);
                         windowBackend->Update();
                     });

    Window* primaryWindow = engineBackend->GetPrimaryWindow();
    windowBackend = new WindowBackend(engineBackend, primaryWindow);
    engineBackend->OnGameLoopStarted();
    timer.start(16.0);

    QObject::connect(&app, &QApplication::applicationStateChanged, [this](Qt::ApplicationState state)
                     {
                         Window* primaryWindow = engineBackend->GetPrimaryWindow();
                         if (primaryWindow == nullptr)
                         {
                             return;
                         }

                         bool isInFocus = false;
                         if (state == Qt::ApplicationActive)
                         {
                             isInFocus = true;
                         }

                         primaryWindow->PostFocusChanged(isInFocus);
                     });

    QObject::connect(&app, &QApplication::aboutToQuit, [this]()
                     {
                         engineBackend->OnGameLoopStopped();
                         engineBackend->OnBeforeTerminate();
                     });

    app.exec();
}

void PlatformCore::Quit()
{
    DVASSERT(false);
}

QApplication* PlatformCore::GetApplication()
{
    return qApp;
}

RenderWidget* PlatformCore::GetRenderWidget()
{
    return windowBackend->GetRenderWidget();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
