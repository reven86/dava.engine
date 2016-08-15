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
    int qtArgc = qtCommandLine.size();
    ;

    QApplication app(qtArgc, qtCommandLine.data());

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]()
                     {
                         DVASSERT(windowBackend != nullptr);
                         RenderWidget* widget = windowBackend->GetRenderWidget();
                         DVASSERT(widget);
                         QQuickWindow* window = widget->quickWindow();
                         DVASSERT(window);
                         if (window->isVisible())
                         {
                             window->update();
                         }
                     });

    windowBackend = CreateNativeWindow(engineBackend->GetPrimaryWindow(), 180.0f, 180.0f);
    if (windowBackend == nullptr)
    {
        return;
    }

    engineBackend->OnGameLoopStarted();
    timer.start(16.0);

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

WindowBackend* PlatformCore::CreateNativeWindow(Window* w, float32 width, float32 height)
{
    WindowBackend* backend = new WindowBackend(engineBackend, w);
    if (!backend->Create(width, height))
    {
        delete backend;
        backend = nullptr;
    }

    return backend;
}

QApplication* PlatformCore::GetApplication()
{
    return qApp;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
