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
PlatformCore::PlatformCore(EngineBackend* engineBackend_)
    : engineBackend(engineBackend_)
    , nativeService(new NativeService(this))
{
}

PlatformCore::~PlatformCore()
{
    for (int32 i = 0; i < argc; ++i)
    {
        delete[] argvMemory[i];
    }
    delete[] argvMemory;
}

void PlatformCore::Init()
{
    const Vector<String>& commandLine = engineBackend->GetCommandLine();
    argc = static_cast<int>(commandLine.size());
    argvMemory = new char8*[argc];
    for (int i = 0; i < argc; ++i)
    {
        const String& arg = commandLine[i];
        size_t size = arg.size();
        argvMemory[i] = new char8[size + 1];
        Memset(argvMemory[i], 0, sizeof(char8) * size + 1);
        Memcpy(argvMemory[i], arg.data(), sizeof(char8) * size);
    }

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setAlphaBufferSize(0);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    application.reset(new QApplication(argc, argvMemory));
}

void PlatformCore::Run()
{
    DVASSERT(application);
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
    application->exec();
    engineBackend->OnGameLoopStopped();
    engineBackend->OnBeforeTerminate();
}

void PlatformCore::Quit()
{
    DVASSERT(application);
    application->quit();
}

DAVA::NativeService* PlatformCore::GetNativeService()
{
    return nativeService.get();
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
    return application.get();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
