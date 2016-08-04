#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Qt/WindowBackendQt.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Public/Window.h"

#include "Engine/Public/Qt/NativeServiceQt.h"
#include "Engine/Public/Qt/WindowNativeServiceQt.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/Qt/WindowBackendQt.h"

#include "UI/UIEvent.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QObject>

namespace DAVA
{
namespace Private
{
class OGLContextBinder : public DAVA::Singleton<OGLContextBinder>
{
public:
    OGLContextBinder(QSurface* surface, QOpenGLContext* context)
        : davaContext(surface, context)
    {
    }

    void AcquireContext()
    {
        QSurface* prevSurface = nullptr;
        QOpenGLContext* prevContext = QOpenGLContext::currentContext();
        if (prevContext != nullptr)
            prevSurface = prevContext->surface();

        contextStack.emplace(prevSurface, prevContext);

        if (prevContext != davaContext.context)
        {
            davaContext.context->makeCurrent(davaContext.surface);
        }
    }

    void ReleaseContext()
    {
        DVASSERT(!contextStack.empty());
        QOpenGLContext* currentContext = QOpenGLContext::currentContext();

        ContextNode topNode = contextStack.top();
        contextStack.pop();

        if (topNode.context == currentContext)
        {
            return;
        }
        else if (currentContext != nullptr)
        {
            currentContext->doneCurrent();
        }

        if (topNode.context != nullptr && topNode.surface != nullptr)
        {
            topNode.context->makeCurrent(topNode.surface);
        }
    }

private:
    struct ContextNode
    {
        ContextNode(QSurface* surface_ = nullptr, QOpenGLContext* context_ = nullptr)
            : surface(surface_)
            , context(context_)
        {
        }

        QSurface* surface = nullptr;
        QOpenGLContext* context = nullptr;
    };

    ContextNode davaContext;
    DAVA::Stack<ContextNode> contextStack;
};

void AcqureContext()
{
    if (OGLContextBinder::Instance())
    {
        OGLContextBinder::Instance()->AcquireContext();
    }
}

void ReleaseContext()
{
    if (OGLContextBinder::Instance())
    {
        OGLContextBinder::Instance()->ReleaseContext();
    }
}

class TriggerProcessEvent : public QEvent
{
public:
    TriggerProcessEvent()
        : QEvent(eventType)
    {
    }

    static const Type eventType = static_cast<Type>(User + 1);
};

class WindowBackend::QtEventListener : public QObject
{
public:
    using TCallback = Function<void()>;
    QtEventListener(const TCallback& triggered_,
                    const TCallback& destroyed_,
                    QObject* parent)
        : triggered(triggered_)
        , destroyed(destroyed_)
    {
    }

    ~QtEventListener()
    {
        destroyed();
    }

protected:
    bool event(QEvent* e) override
    {
        if (e->type() == TriggerProcessEvent::eventType)
        {
            triggered();
            return true;
        }

        return false;
    }

private:
    TCallback triggered;
    TCallback destroyed;
};

WindowBackend::WindowBackend(EngineBackend* e, Window* w)
    : engine(e)
    , dispatcher(engine->GetDispatcher())
    , window(w)
    , platformDispatcher(MakeFunction(this, &WindowBackend::PlatformEventHandler))
    , nativeService(new WindowNativeService(this))
{
    QtEventListener::TCallback triggered = [this]()
    {
        platformDispatcher.ProcessEvents();
    };

    QtEventListener::TCallback destroyed = [this]()
    {
        qtEventListener = nullptr;
    };

    qtEventListener = new QtEventListener(triggered, destroyed, engine->GetNativeService()->GetApplication());
}

WindowBackend::~WindowBackend()
{
    delete renderWidget;
}

void* WindowBackend::GetHandle() const
{
    return nullptr;
}

WindowNativeService* WindowBackend::GetNativeService() const
{
    return nativeService.get();
}

bool WindowBackend::Create(float32 width, float32 height)
{
    renderWidget = new RenderWidget(this, static_cast<uint32>(width), static_cast<uint32>(height));
    renderWidget->show();
    return true;
}

void WindowBackend::Resize(float32 width, float32 height)
{
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::RESIZE_WINDOW;
    e.resizeEvent.width = width;
    e.resizeEvent.height = height;
    platformDispatcher.PostEvent(e);
}

void WindowBackend::Close()
{
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::CLOSE_WINDOW;
    platformDispatcher.PostEvent(e);
}

void WindowBackend::RunAsyncOnUIThread(const Function<void()>& task)
{
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::FUNCTOR;
    e.functor = task;
    platformDispatcher.PostEvent(e);
}

void WindowBackend::TriggerPlatformEvents()
{
    NativeService* service = engine->GetNativeService();
    DVASSERT(service);
    QApplication* app = service->GetApplication();
    DVASSERT(app);
    if (app)
    {
        app->postEvent(qtEventListener, new TriggerProcessEvent());
    }
}

void WindowBackend::PlatformEventHandler(const UIDispatcherEvent& e)
{
    switch (e.type)
    {
    case UIDispatcherEvent::RESIZE_WINDOW:
        DoResizeWindow(e.resizeEvent.width, e.resizeEvent.height);
        break;
    case UIDispatcherEvent::CLOSE_WINDOW:
        DoCloseWindow();
        break;
    case UIDispatcherEvent::FUNCTOR:
        e.functor();
        break;
    default:
        break;
    }
}

namespace WindowBackendDetails
{
//there is a bug in Qt: https://bugreports.qt.io/browse/QTBUG-50465
void Kostil_ForceUpdateCurrentScreen(RenderWidget* renderWidget, QApplication* application)
{
    QDesktopWidget* desktop = application->desktop();
    int screenNumber = desktop->screenNumber(renderWidget);
    DVASSERT(screenNumber >= 0 && screenNumber < qApp->screens().size());

    QWindow* parent = renderWidget->quickWindow();
    while (parent->parent() != nullptr)
    {
        parent = parent->parent();
    }
    parent->setScreen(application->screens().at(screenNumber));
}
} //unnamed namespace

void WindowBackend::OnCreated()
{
    new OGLContextBinder(renderWidget->quickWindow(), renderWidget->quickWindow()->openglContext());

    WindowBackendDetails::Kostil_ForceUpdateCurrentScreen(renderWidget, engine->GetNativeService()->GetApplication());
    float32 dpi = renderWidget->devicePixelRatioF();
    window->PostWindowCreated(this, renderWidget->width(), renderWidget->height(), dpi, dpi);
}

void WindowBackend::OnDestroyed()
{
    OGLContextBinder::Instance()->Release();
    renderWidget = nullptr;
}

void WindowBackend::OnFrame()
{
    engine->OnFrame();
}

void WindowBackend::OnResized(uint32 width, uint32 height, float32 dpi)
{
    window->PostSizeChanged(static_cast<float32>(width), static_cast<float32>(height), dpi, dpi);
}

void WindowBackend::OnVisibilityChanged(bool isVisible)
{
    window->PostVisibilityChanged(isVisible);
    window->PostFocusChanged(isVisible);
}

void WindowBackend::OnMousePressed(QMouseEvent* qtEvent)
{
    MainDispatcherEvent e;
    e.timestamp = qtEvent->timestamp();
    e.window = window;
    e.type = MainDispatcherEvent::MOUSE_BUTTON_DOWN;
    e.mclickEvent.clicks = 1;
    e.mclickEvent.button = ConvertButtons(qtEvent->button());
    e.mclickEvent.x = DpiConvert(qtEvent->x());
    e.mclickEvent.y = DpiConvert(qtEvent->y());
    dispatcher->PostEvent(e);
}

void WindowBackend::OnMouseReleased(QMouseEvent* qtEvent)
{
    MainDispatcherEvent e;
    e.timestamp = qtEvent->timestamp();
    e.window = window;
    e.type = MainDispatcherEvent::MOUSE_BUTTON_UP;
    e.mclickEvent.clicks = 1;
    e.mclickEvent.button = ConvertButtons(qtEvent->button());
    e.mclickEvent.x = DpiConvert(qtEvent->x());
    e.mclickEvent.y = DpiConvert(qtEvent->y());
    dispatcher->PostEvent(e);
}

void WindowBackend::OnMouseMove(QMouseEvent* qtEvent)
{
    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::MOUSE_MOVE;
    e.timestamp = qtEvent->timestamp();
    e.window = window;
    e.mmoveEvent.x = DpiConvert(qtEvent->x());
    e.mmoveEvent.y = DpiConvert(qtEvent->y());
    dispatcher->PostEvent(e);
}

void WindowBackend::OnMouseDBClick(QMouseEvent* qtEvent)
{
    MainDispatcherEvent e;
    e.timestamp = qtEvent->timestamp();
    e.window = window;
    e.type = MainDispatcherEvent::MOUSE_BUTTON_UP;
    e.mclickEvent.clicks = 2;
    e.mclickEvent.button = ConvertButtons(qtEvent->button());
    e.mclickEvent.x = DpiConvert(qtEvent->x());
    e.mclickEvent.y = DpiConvert(qtEvent->y());
    dispatcher->PostEvent(e);
}

void WindowBackend::OnWheel(QWheelEvent* qtEvent)
{
    if (qtEvent->phase() != Qt::ScrollUpdate)
    {
        return;
    }

    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::MOUSE_WHEEL;
    e.timestamp = qtEvent->timestamp();
    e.window = window;
    e.mwheelEvent.x = DpiConvert(qtEvent->x());
    e.mwheelEvent.y = DpiConvert(qtEvent->y());
    QPoint pixelDelta = qtEvent->pixelDelta();
    if (!pixelDelta.isNull())
    {
        e.mwheelEvent.deltaX = static_cast<float32>(qtEvent->pixelDelta().x());
        e.mwheelEvent.deltaY = static_cast<float32>(qtEvent->pixelDelta().y());
    }
    else
    {
        QPointF delta = QPointF(qtEvent->angleDelta()) / 180.0f;
        e.mwheelEvent.deltaX = delta.x();
        e.mwheelEvent.deltaY = delta.y();
    }
    dispatcher->PostEvent(e);
}

void WindowBackend::OnKeyPressed(QKeyEvent* qtEvent)
{
#ifdef Q_OS_WIN
    uint32 nativeModif = qtEvent->nativeModifiers();
    uint32 nativeScanCode = qtEvent->nativeScanCode();
    uint32 virtKey = qtEvent->nativeVirtualKey();
    if ((1 << 24) & nativeModif)
    {
        virtKey |= 0x100;
    }
    if (VK_SHIFT == virtKey && nativeScanCode == 0x36) // is right shift key
    {
        virtKey |= 0x100;
    }
#else
    uint32 virtKey = qtEvent->nativeVirtualKey();
    if (virtKey == 0)
    {
        virtKey = ConvertQtCommandKeysToDava(qtEvent->key());
    }
#endif

    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::KEY_DOWN;
    e.timestamp = qtEvent->timestamp();
    e.window = window;
    e.keyEvent.key = virtKey;
    e.keyEvent.isRepeated = qtEvent->isAutoRepeat();
    dispatcher->PostEvent(e);

    QString text = qtEvent->text();
    for (int i = 0; i < text.size(); ++i)
    {
        QCharRef charRef = text[i];
        MainDispatcherEvent e;
        e.type = MainDispatcherEvent::KEY_CHAR;
        e.window = window;
        e.timestamp = qtEvent->timestamp();
        e.keyEvent.key = charRef.unicode();
        e.keyEvent.isRepeated = qtEvent->isAutoRepeat();
        dispatcher->PostEvent(e);
    }
}

void WindowBackend::OnKeyReleased(QKeyEvent* qtEvent)
{
#ifdef Q_OS_WIN
    uint32 nativeModif = qtEvent->nativeModifiers();
    uint32 nativeScanCode = qtEvent->nativeScanCode();
    uint32 virtKey = qtEvent->nativeVirtualKey();
    if ((1 << 24) & nativeModif)
    {
        virtKey |= 0x100;
    }
    if (VK_SHIFT == virtKey && nativeScanCode == 0x36) // is right shift key
    {
        virtKey |= 0x100;
    }
#else
    qint32 virtKey = qtEvent->nativeVirtualKey();
    if (virtKey == 0)
    {
        virtKey = ConvertQtCommandKeysToDava(qtEvent->key());
    }
#endif

    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::KEY_UP;
    e.timestamp = qtEvent->timestamp();
    e.window = window;
    e.keyEvent.key = virtKey;
    e.keyEvent.isRepeated = qtEvent->isAutoRepeat();
    dispatcher->PostEvent(e);
}

void WindowBackend::DoResizeWindow(float32 width, float32 height)
{
    DVASSERT(renderWidget);
    renderWidget->resize(width, height);
}

void WindowBackend::DoCloseWindow()
{
    // i don't know what i can do here
    // renderWidget->hide() ???
}

DAVA::RenderWidget* WindowBackend::GetRenderWidget()
{
    return renderWidget;
}

void WindowBackend::InitRenderParams(rhi::InitParam& params)
{
    params.threadedRenderEnabled = false;
    params.threadedRenderFrameCount = 1;
    params.acquireContextFunc = &AcqureContext;
    params.releaseContextFunc = &ReleaseContext;
}

uint32 WindowBackend::ConvertButtons(Qt::MouseButton button)
{
    UIEvent::MouseButton mouseButton = UIEvent::MouseButton::NONE;

    if (button == Qt::LeftButton)
    {
        mouseButton = UIEvent::MouseButton::LEFT;
    }
    if (button == Qt::RightButton)
    {
        mouseButton = UIEvent::MouseButton::RIGHT;
    }
    if (button == Qt::MiddleButton)
    {
        mouseButton = UIEvent::MouseButton::MIDDLE;
    }
    if (button == Qt::XButton1)
    {
        mouseButton = UIEvent::MouseButton::EXTENDED1;
    }
    if (button == Qt::XButton2)
    {
        mouseButton = UIEvent::MouseButton::EXTENDED2;
    }

    return static_cast<uint32>(mouseButton);
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
