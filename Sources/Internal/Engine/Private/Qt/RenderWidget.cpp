#include "Engine/Qt/RenderWidget.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"
#include "Base/Exception.h"

#if defined(__DAVAENGINE_QT__)

#include "Base/Exception.h"
#include "Debug/DVAssert.h"

#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

#include "Logger/Logger.h"

#include <QQuickWindow>
#include <QQuickWidget>
#include <QQuickItem>
#include <QOpenGLContext>
#include <QVariant>
#include <QVBoxLayout>

namespace DAVA
{
namespace RenderWidgetDetails
{
struct QtScreenParams
{
    int screenScale = 0;
    int logicalDPI = 0;
};
}

const char* initializedPropertyName = "initialized";

class RenderWidget::RenderWidgetImpl : public QQuickWidget
{
public:
    RenderWidgetImpl(RenderWidget::IWindowDelegate* widgetDelegate_, uint32 width, uint32 height);

    void ActivateRendering();
    bool IsInitialized() const;

    void SetClientDelegate(IClientDelegate* delegate);
    Signal<uint32, uint32> resized;

protected:
    bool eventFilter(QObject* object, QEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;
    void closeEvent(QCloseEvent* e) override;
    void timerEvent(QTimerEvent* e) override;
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dragLeaveEvent(QDragLeaveEvent* e) override;
    void dropEvent(QDropEvent* e) override;
    void dragMoveEvent(QDragMoveEvent* e) override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;

    bool event(QEvent* e) override;

private:
    void TryActivate();
    void OnCreated();
    void OnFrame();
    void OnActiveFocusItemChanged();
    void OnSceneGraphInvalidated();
    void OnClientDelegateDestroyed();
    void OnBeforeSyncronizing();

private:
    IWindowDelegate* widgetDelegate = nullptr;
    IClientDelegate* clientDelegate = nullptr;
    bool keyEventRecursiveGuard = false;

    bool isClosing = false;
    bool isInPaint = false;

    bool isSynchronized = false;
    bool isActivated = false;

    std::unique_ptr<RenderWidgetDetails::QtScreenParams> screenParams;
};

RenderWidget::RenderWidgetImpl::RenderWidgetImpl(RenderWidget::IWindowDelegate* widgetDelegate_, uint32 width, uint32 height)
    : widgetDelegate(widgetDelegate_)
{
    setAcceptDrops(true);
    setMouseTracking(true);

    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setMinimumSize(QSize(width, height));
    setResizeMode(QQuickWidget::SizeViewToRootObject);

    QQuickWindow* window = quickWindow();
    window->installEventFilter(this);
    window->setClearBeforeRendering(true);
    window->setColor(QColor(76, 76, 76, 255));
    connect(window, &QQuickWindow::sceneGraphInvalidated, this, &RenderWidgetImpl::OnSceneGraphInvalidated, Qt::DirectConnection);
    connect(window, &QQuickWindow::activeFocusItemChanged, this, &RenderWidgetImpl::OnActiveFocusItemChanged, Qt::DirectConnection);

    connect(window, &QQuickWindow::beforeSynchronizing, this, &RenderWidgetImpl::OnBeforeSyncronizing, Qt::DirectConnection);
}

void RenderWidget::RenderWidgetImpl::TryActivate()
{
    if (IsInitialized() == false && isActivated && isSynchronized)
    {
        ActivateRendering();
        OnCreated();
    }
}

void RenderWidget::RenderWidgetImpl::OnCreated()
{
    setProperty(initializedPropertyName, true);

    widgetDelegate->OnCreated();

    screenParams = std::make_unique<RenderWidgetDetails::QtScreenParams>();
    screenParams->screenScale = devicePixelRatio();
    screenParams->logicalDPI = logicalDpiX();

    QSize size = geometry().size();
    QQuickWindow* qWindow = quickWindow();
    bool isFullScreen = qWindow != nullptr ? qWindow->visibility() == QWindow::FullScreen : false;

    widgetDelegate->OnResized(size.width(), size.height(), isFullScreen);
    resized.Emit(size.width(), size.height());
}

void RenderWidget::RenderWidgetImpl::OnFrame()
{
    DVASSERT(isInPaint == false);
    isInPaint = true;
    SCOPE_EXIT
    {
        isInPaint = false;
    };

    //process screen changing of screens or screens params outside the app
    DVASSERT(screenParams);
    if (screenParams->screenScale != devicePixelRatio())
    {
        screenParams->screenScale = devicePixelRatio();

        QQuickWindow* qWindow = quickWindow();
        bool isFullScreen = qWindow != nullptr ? qWindow->visibility() == QWindow::FullScreen : false;

        QSize size = geometry().size();
        widgetDelegate->OnResized(size.width(), size.height(), isFullScreen);
    }

    if (screenParams->logicalDPI != logicalDpiX())
    {
        screenParams->logicalDPI = logicalDpiX();
        widgetDelegate->OnDpiChanged(static_cast<float32>(screenParams->logicalDPI));
    }

    QVariant nativeHandle = quickWindow()->openglContext()->nativeHandle();
    if (!nativeHandle.isValid())
    {
        DAVA::Logger::Error("GL context is not valid!");
        DAVA_THROW(DAVA::Exception, "GL context is not valid!");
    }

    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    widgetDelegate->OnFrame();
    quickWindow()->resetOpenGLState();
}

void RenderWidget::RenderWidgetImpl::ActivateRendering()
{
    QQuickWindow* w = quickWindow();
    connect(w, &QQuickWindow::beforeRendering, this, &RenderWidgetImpl::OnFrame, Qt::DirectConnection);
    w->setClearBeforeRendering(false);
}

bool RenderWidget::RenderWidgetImpl::IsInitialized() const
{
    return property(initializedPropertyName).isValid();
}

void RenderWidget::RenderWidgetImpl::SetClientDelegate(RenderWidget::IClientDelegate* delegate)
{
    DVASSERT(nullptr == clientDelegate);
    clientDelegate = delegate;
    QObject* qobjectDelegate = dynamic_cast<QObject*>(delegate);
    if (qobjectDelegate != nullptr)
    {
        QObject::connect(qobjectDelegate, &QObject::destroyed, this, &RenderWidgetImpl::OnClientDelegateDestroyed);
    }
}

void RenderWidget::RenderWidgetImpl::OnActiveFocusItemChanged()
{
    QQuickItem* item = quickWindow()->activeFocusItem();
    bool focusRequested = item != nullptr;
    if (focusRequested)
    {
        item->installEventFilter(this);
    }

    KeyboardDevice& kd = InputSystem::Instance()->GetKeyboard();
    kd.ClearAllKeys(); //we need only reset keyboard status on focus changing
}

void RenderWidget::RenderWidgetImpl::OnSceneGraphInvalidated()
{
    if (isClosing)
    {
        widgetDelegate->OnDestroyed();
    }
}

void RenderWidget::RenderWidgetImpl::resizeEvent(QResizeEvent* e)
{
    QQuickWidget::resizeEvent(e);
    QSize size = e->size();

    QQuickWindow* qWindow = quickWindow();
    bool isFullScreen = qWindow != nullptr ? qWindow->visibility() == QWindow::FullScreen : false;

    widgetDelegate->OnResized(size.width(), size.height(), isFullScreen);
    resized.Emit(size.width(), size.height());
}

void RenderWidget::RenderWidgetImpl::showEvent(QShowEvent* e)
{
    QQuickWidget::showEvent(e);
    widgetDelegate->OnVisibilityChanged(true);
}

void RenderWidget::RenderWidgetImpl::hideEvent(QHideEvent* e)
{
    widgetDelegate->OnVisibilityChanged(false);
    QQuickWidget::hideEvent(e);
}

void RenderWidget::RenderWidgetImpl::closeEvent(QCloseEvent* e)
{
    if (widgetDelegate->OnUserCloseRequest())
    {
        isClosing = true;
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void RenderWidget::RenderWidgetImpl::timerEvent(QTimerEvent* e)
{
    QQuickWidget::timerEvent(e);
}

void RenderWidget::RenderWidgetImpl::dragEnterEvent(QDragEnterEvent* e)
{
    if (clientDelegate != nullptr)
    {
        clientDelegate->OnDragEntered(e);
    }
}

void RenderWidget::RenderWidgetImpl::dragMoveEvent(QDragMoveEvent* e)
{
    widgetDelegate->OnDragMoved(e);
    if (clientDelegate != nullptr)
    {
        clientDelegate->OnDragMoved(e);
    }
}

void RenderWidget::RenderWidgetImpl::dragLeaveEvent(QDragLeaveEvent* e)
{
    if (clientDelegate != nullptr)
    {
        clientDelegate->OnDragLeaved(e);
    }
}

void RenderWidget::RenderWidgetImpl::dropEvent(QDropEvent* e)
{
    if (clientDelegate != nullptr)
    {
        clientDelegate->OnDrop(e);
    }
}

void RenderWidget::RenderWidgetImpl::mousePressEvent(QMouseEvent* e)
{
    QQuickWidget::mousePressEvent(e);
    widgetDelegate->OnMousePressed(e);

    if (clientDelegate != nullptr)
    {
        clientDelegate->OnMousePressed(e);
    }
}

void RenderWidget::RenderWidgetImpl::mouseReleaseEvent(QMouseEvent* e)
{
    QQuickWidget::mouseReleaseEvent(e);
    widgetDelegate->OnMouseReleased(e);

    if (clientDelegate != nullptr)
    {
        clientDelegate->OnMouseReleased(e);
    }
}

void RenderWidget::RenderWidgetImpl::mouseDoubleClickEvent(QMouseEvent* e)
{
    QQuickWidget::mouseDoubleClickEvent(e);
    widgetDelegate->OnMouseDBClick(e);

    if (clientDelegate != nullptr)
    {
        clientDelegate->OnMouseDBClick(e);
    }
}

void RenderWidget::RenderWidgetImpl::mouseMoveEvent(QMouseEvent* e)
{
    QQuickWidget::mouseMoveEvent(e);
    widgetDelegate->OnMouseMove(e);

    if (clientDelegate != nullptr)
    {
        clientDelegate->OnMouseMove(e);
    }
}

void RenderWidget::RenderWidgetImpl::wheelEvent(QWheelEvent* e)
{
    QQuickWidget::wheelEvent(e);
    widgetDelegate->OnWheel(e);

    if (clientDelegate != nullptr)
    {
        clientDelegate->OnWheel(e);
    }
}

void RenderWidget::RenderWidgetImpl::keyPressEvent(QKeyEvent* e)
{
    QQuickWidget::keyPressEvent(e);
    widgetDelegate->OnKeyPressed(e);

    if (clientDelegate != nullptr)
    {
        clientDelegate->OnKeyPressed(e);
    }
}

void RenderWidget::RenderWidgetImpl::keyReleaseEvent(QKeyEvent* e)
{
    QQuickWidget::keyReleaseEvent(e);
    widgetDelegate->OnKeyReleased(e);
    if (clientDelegate != nullptr)
    {
        clientDelegate->OnKeyReleased(e);
    }
}

bool RenderWidget::RenderWidgetImpl::event(QEvent* e)
{
    QEvent::Type eventType = e->type();
    if (eventType == QEvent::NativeGesture && clientDelegate != nullptr)
    {
        QNativeGestureEvent* gestureEvent = static_cast<QNativeGestureEvent*>(e);
        widgetDelegate->OnNativeGesture(gestureEvent);
        if (clientDelegate != nullptr)
        {
            clientDelegate->OnNativeGesture(gestureEvent);
        }
    }
    if (eventType == QEvent::WindowActivate || (eventType == QEvent::Polish && isActiveWindow()))
    {
        isActivated = true;
        TryActivate();
    }

    return QQuickWidget::event(e);
}

bool RenderWidget::RenderWidgetImpl::eventFilter(QObject* object, QEvent* e)
{
    QEvent::Type t = e->type();
    if ((t == QEvent::KeyPress || t == QEvent::KeyRelease) && keyEventRecursiveGuard == false)
    {
        QQuickItem* focusObject = quickWindow()->activeFocusItem();
        if (object == quickWindow() || object == focusObject)
        {
            keyEventRecursiveGuard = true;
            SCOPE_EXIT
            {
                keyEventRecursiveGuard = false;
            };
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
            if (t == QEvent::KeyPress)
            {
                keyPressEvent(keyEvent);
            }
            else
            {
                keyReleaseEvent(keyEvent);
            }
            return true;
        }
    }

    return false;
}

void RenderWidget::RenderWidgetImpl::OnClientDelegateDestroyed()
{
    clientDelegate = nullptr;
}

void RenderWidget::RenderWidgetImpl::OnBeforeSyncronizing()
{
    disconnect(quickWindow(), &QQuickWindow::beforeSynchronizing, this, &RenderWidgetImpl::OnBeforeSyncronizing);
    isSynchronized = true;
    TryActivate();
}

/////////////////////////////////////////////////////////////////////////////////
//                      RenderWidget                                           //
/////////////////////////////////////////////////////////////////////////////////

RenderWidget::RenderWidget(RenderWidget::IWindowDelegate* widgetDelegate, uint32 width, uint32 height)
    : impl(new RenderWidgetImpl(widgetDelegate, width, height))
{
    QVBoxLayout* boxLayout = new QVBoxLayout(this);
    boxLayout->setSpacing(0);
    boxLayout->setMargin(0);
    boxLayout->addWidget(impl);
    setLayout(boxLayout);

    impl->resized.Connect(&resized, &Signal<uint32, uint32>::Emit);
}

RenderWidget::~RenderWidget() = default;

void RenderWidget::SetClientDelegate(RenderWidget::IClientDelegate* delegate)
{
    impl->SetClientDelegate(delegate);
}

bool RenderWidget::IsInitialized() const
{
    return impl->IsInitialized();
}

QQuickWindow* RenderWidget::GetQQuickWindow()
{
    return impl->quickWindow();
}

} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
