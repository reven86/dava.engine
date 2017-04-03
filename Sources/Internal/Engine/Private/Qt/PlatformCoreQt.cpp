
#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Qt/PlatformCoreQt.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Window.h"
#include "Engine/Qt/RenderWidget.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/WindowBackend.h"

#include <QTimer>
#include <QApplication>
#include <QSurfaceFormat>

namespace DAVA
{
namespace Private
{
PlatformCore::PlatformCore(EngineBackend* engineBackend)
    : engineBackend(*engineBackend)
{
}

PlatformCore::~PlatformCore() = default;

void PlatformCore::Init()
{
    engineBackend.InitializePrimaryWindow();
}

void PlatformCore::Run()
{
    Vector<char*> qtCommandLine = engineBackend.GetCommandLineAsArgv();
    int qtArgc = static_cast<int>(qtCommandLine.size());

    QApplication app(qtArgc, qtCommandLine.data());
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setAlphaBufferSize(0);
    QSurfaceFormat::setDefaultFormat(format);

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]()
                     {
                         if (!EngineBackend::showingModalMessageBox)
                         {
                             DVASSERT(primaryWindowBackend != nullptr);
                             primaryWindowBackend->Update();
                         }
                     });

    // First of all we should init primaryWindowBackend, because in OnGameLoopStarted client code will try to get RenderWidget trough this pointer
    primaryWindowBackend = EngineBackend::GetWindowBackend(engineBackend.GetPrimaryWindow());
    engineBackend.OnGameLoopStarted();
    applicationFocusChanged.Connect(primaryWindowBackend, &WindowBackend::OnApplicationFocusChanged);
    if (engineBackend.IsStandaloneGUIMode())
    {
        // Force RenderWidget creation and show it on screen
        RenderWidget* widget = GetRenderWidget();
        widget->show();
    }

    timer.start(16.0);

    QObject::connect(&app, &QApplication::applicationStateChanged, [this](Qt::ApplicationState state) {
        applicationFocusChanged.Emit(state == Qt::ApplicationActive);
    });

    QObject::connect(&app, &QApplication::aboutToQuit, [this]() {
        engineBackend.OnGameLoopStopped();
        engineBackend.OnEngineCleanup();
    });

    app.exec();
}

void PlatformCore::PrepareToQuit()
{
    engineBackend.PostAppTerminate(true);
}

void PlatformCore::Quit()
{
    // Do nothing as application is terminated when window has closed.
    // In embedded mode this method should not be invoked
    DVASSERT(engineBackend.IsEmbeddedGUIMode() == false);
}

void PlatformCore::SetScreenTimeoutEnabled(bool enabled)
{
    // TODO
}

QApplication* PlatformCore::GetApplication()
{
    return qApp;
}

RenderWidget* PlatformCore::GetRenderWidget()
{
    return primaryWindowBackend->GetRenderWidget();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
