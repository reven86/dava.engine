#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Qt/PlatformCoreQt.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Public/Qt/NativeServiceQt.h"
#include "Engine/Public/Qt/RenderWidget.h"

#include <QTimer>
#include <QApplication>

namespace DAVA
{
namespace Private
{
std::unique_ptr<QApplication> globalApplication;
Vector<char*> qtCommandLine;
int qtArgc = 0;

PlatformCore::PlatformCore(EngineBackend* engineBackend_)
    : engineBackend(engineBackend_)
    , nativeService(new NativeService(this))
{
}

PlatformCore::~PlatformCore() = default;

void PlatformCore::Init()
{
    DVASSERT(globalApplication == nullptr);
    qtCommandLine = engineBackend->GetCommandLineAsArgv();
    qtArgc = static_cast<int>(qtCommandLine.size());
    globalApplication.reset(new QApplication(qtArgc, qtCommandLine.data()));
}

void PlatformCore::Run()
{
    DVASSERT(globalApplication);
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

    engineBackend->OnGameLoopStarted();
    windowBackend = CreateNativeWindow(engineBackend->GetPrimaryWindow(), 640.0f, 480.0f);
    if (windowBackend == nullptr)
    {
        return;
    }
    timer.start(16.0);

    QObject::connect(globalApplication.get(), &QApplication::aboutToQuit, [this]()
                     {
                         engineBackend->OnGameLoopStopped();
                         engineBackend->OnBeforeTerminate();
                     });

    globalApplication->exec();
}

void PlatformCore::Quit()
{
    DVASSERT(globalApplication);
    globalApplication->quit();
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
    return globalApplication.get();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
