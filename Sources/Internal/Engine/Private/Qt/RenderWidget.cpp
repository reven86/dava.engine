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
#include <QQuickItem>
#include <QOpenGLContext>
#include <QVariant>

namespace DAVA
{
struct RenderWidget::QtScreenParams
{
    int screenScale = 0;
    int logicalDPI = 0;
};

const char* initializedPropertyName = "initialized";
RenderWidget::RenderWidget(RenderWidget::IWindowDelegate* widgetDelegate_, uint32 width, uint32 height)
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
    connect(window, &QQuickWindow::sceneGraphInvalidated, this, &RenderWidget::OnSceneGraphInvalidated, Qt::DirectConnection);
    connect(window, &QQuickWindow::activeFocusItemChanged, this, &RenderWidget::OnActiveFocusItemChanged, Qt::DirectConnection);
}

RenderWidget::~RenderWidget() = default;

void RenderWidget::SetClientDelegate(RenderWidget::IClientDelegate* delegate)
{
    DVASSERT(nullptr == clientDelegate);
    clientDelegate = delegate;
    QObject* qobjectDelegate = dynamic_cast<QObject*>(delegate);
    if (qobjectDelegate != nullptr)
    {
        QObject::connect(qobjectDelegate, &QObject::destroyed, this, &RenderWidget::OnClientDelegateDestroyed);
    }
}

void RenderWidget::OnCreated()
{
    QObject::disconnect(quickWindow(), &QQuickWindow::beforeSynchronizing, this, &RenderWidget::OnCreated);
    setProperty(initializedPropertyName, true);

    widgetDelegate->OnCreated();

    screenParams = std::make_unique<QtScreenParams>();
    screenParams->screenScale = devicePixelRatio();
    screenParams->logicalDPI = logicalDpiX();

    QSize size = geometry().size();
    QQuickWindow* qWindow = quickWindow();
    bool isFullScreen = qWindow != nullptr ? qWindow->visibility() == QWindow::FullScreen : false;

    widgetDelegate->OnResized(size.width(), size.height(), isFullScreen);
    emit Resized(size.width(), size.height());
}

void RenderWidget::OnFrame()
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

void RenderWidget::ActivateRendering()
{
    QQuickWindow* w = quickWindow();
    connect(w, &QQuickWindow::beforeSynchronizing, this, &RenderWidget::OnCreated, Qt::DirectConnection);
    connect(w, &QQuickWindow::beforeRendering, this, &RenderWidget::OnFrame, Qt::DirectConnection);
    w->setClearBeforeRendering(false);
}

bool RenderWidget::IsInitialized() const
{
    return property(initializedPropertyName).isValid();
}

void RenderWidget::OnActiveFocusItemChanged()
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

void RenderWidget::OnSceneGraphInvalidated()
{
    if (isClosing)
    {
        widgetDelegate->OnDestroyed();
    }
}

void RenderWidget::resizeEvent(QResizeEvent* e)
{
    QQuickWidget::resizeEvent(e);
    QSize size = e->size();

    QQuickWindow* qWindow = quickWindow();
    bool isFullScreen = qWindow != nullptr ? qWindow->visibility() == QWindow::FullScreen : false;

    widgetDelegate->OnResized(size.width(), size.height(), isFullScreen);
    emit Resized(size.width(), size.height());
}

void RenderWidget::showEvent(QShowEvent* e)
{
    QQuickWidget::showEvent(e);
    widgetDelegate->OnVisibilityChanged(true);
}

void RenderWidget::hideEvent(QHideEvent* e)
{
    widgetDelegate->OnVisibilityChanged(false);
    QQuickWidget::hideEvent(e);
}

void RenderWidget::closeEvent(QCloseEvent* e)
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

void RenderWidget::timerEvent(QTimerEvent* e)
{
    QQuickWidget::timerEvent(e);
}

void RenderWidget::dragEnterEvent(QDragEnterEvent* e)
{
    if (clientDelegate != nullptr)
    {
        clientDelegate->OnDragEntered(e);
    }
}

void RenderWidget::dragMoveEvent(QDragMoveEvent* e)
{
    widgetDelegate->OnDragMoved(e);
    if (clientDelegate != nullptr)
    {
        clientDelegate->OnDragMoved(e);
    }
}

void RenderWidget::dragLeaveEvent(QDragLeaveEvent* e)
{
    if (clientDelegate != nullptr)
    {
        clientDelegate->OnDragLeaved(e);
    }
}

void RenderWidget::dropEvent(QDropEvent* e)
{
    if (clientDelegate != nullptr)
    {
        clientDelegate->OnDrop(e);
    }
}

void RenderWidget::mousePressEvent(QMouseEvent* e)
{
    QQuickWidget::mousePressEvent(e);
    widgetDelegate->OnMousePressed(e);

    if (clientDelegate != nullptr)
    {
        clientDelegate->OnMousePressed(e);
    }
}

void RenderWidget::mouseReleaseEvent(QMouseEvent* e)
{
    QQuickWidget::mouseReleaseEvent(e);
    widgetDelegate->OnMouseReleased(e);

    if (clientDelegate != nullptr)
    {
        clientDelegate->OnMouseReleased(e);
    }
}

void RenderWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
    QQuickWidget::mouseDoubleClickEvent(e);
    widgetDelegate->OnMouseDBClick(e);

    if (clientDelegate != nullptr)
    {
        clientDelegate->OnMouseDBClick(e);
    }
}

void RenderWidget::mouseMoveEvent(QMouseEvent* e)
{
    QQuickWidget::mouseMoveEvent(e);
    widgetDelegate->OnMouseMove(e);

    if (clientDelegate != nullptr)
    {
        clientDelegate->OnMouseMove(e);
    }
}

void RenderWidget::wheelEvent(QWheelEvent* e)
{
    QQuickWidget::wheelEvent(e);
    widgetDelegate->OnWheel(e);

    if (clientDelegate != nullptr)
    {
        clientDelegate->OnWheel(e);
    }
}

void RenderWidget::keyPressEvent(QKeyEvent* e)
{
    QQuickWidget::keyPressEvent(e);
    widgetDelegate->OnKeyPressed(e);

    if (clientDelegate != nullptr)
    {
        clientDelegate->OnKeyPressed(e);
    }
}

void RenderWidget::keyReleaseEvent(QKeyEvent* e)
{
    QQuickWidget::keyReleaseEvent(e);
    widgetDelegate->OnKeyReleased(e);
    if (clientDelegate != nullptr)
    {
        clientDelegate->OnKeyReleased(e);
    }
}

bool RenderWidget::event(QEvent* e)
{
    if (e->type() == QEvent::NativeGesture && clientDelegate != nullptr)
    {
        QNativeGestureEvent* gestureEvent = static_cast<QNativeGestureEvent*>(e);
        clientDelegate->OnNativeGuesture(gestureEvent);
    }

    return QQuickWidget::event(e);
}

bool RenderWidget::eventFilter(QObject* object, QEvent* e)
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

void RenderWidget::OnClientDelegateDestroyed()
{
    clientDelegate = nullptr;
}

} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
