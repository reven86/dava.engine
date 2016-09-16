#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Qt/PlatformCoreQt.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Qt/NativeServiceQt.h"
#include "Engine/Qt/RenderWidget.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Window.h"

#include <QTimer>
#include <QApplication>
#include <QSurfaceFormat>

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
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setAlphaBufferSize(0);
    QSurfaceFormat::setDefaultFormat(format);

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]()
                     {
                         DVASSERT(windowBackend != nullptr);
                         windowBackend->Update();
                     });

    Window* primaryWindow = engineBackend->GetPrimaryWindow();
    windowBackend = new WindowBackend(engineBackend, primaryWindow);
    applicationFocusChanged.Connect(windowBackend, &WindowBackend::OnApplicationFocusChanged);
    engineBackend->OnGameLoopStarted();
    windowBackend->ActivateRendering();
    timer.start(16.0);

    QObject::connect(&app, &QApplication::applicationStateChanged, [this](Qt::ApplicationState state)
                     {
                         applicationFocusChanged.Emit(state == Qt::ApplicationActive);
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
