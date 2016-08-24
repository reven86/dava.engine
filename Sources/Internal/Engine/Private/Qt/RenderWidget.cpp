#include "Engine/Qt/RenderWidget.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)

#include "Logger/Logger.h"

#include <QQuickWindow>
#include <QOpenGLContext>

namespace DAVA
{
RenderWidget::RenderWidget(RenderWidget::Delegate* widgetDelegate_, uint32 width, uint32 height)
    : widgetDelegate(widgetDelegate_)
{
//configure Qt Scene Graph to single thread mode
#ifdef Q_OS_WIN
    _putenv_s("QSG_RENDER_LOOP", "basic");
#else
    setenv("QSG_RENDER_LOOP", "basic", 1);
#endif

    setAcceptDrops(true);
    setMouseTracking(true);

    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setMinimumSize(QSize(width, height));
    setResizeMode(QQuickWidget::SizeViewToRootObject);

    QQuickWindow* window = quickWindow();
    window->installEventFilter(this);
    window->setClearBeforeRendering(false);
    connect(window, &QQuickWindow::beforeRendering, this, &RenderWidget::OnFrame, Qt::DirectConnection);
}

RenderWidget::~RenderWidget()
{
    widgetDelegate->OnDestroyed();
}

void RenderWidget::OnFrame()
{
    QVariant nativeHandle = quickWindow()->openglContext()->nativeHandle();
    if (!nativeHandle.isValid())
    {
        DAVA::Logger::Error("GL context is not valid!");
        throw std::runtime_error("GL context is not valid!");
    }

    if (initialized == false)
    {
        widgetDelegate->OnCreated();
        initialized = true;
    }

    widgetDelegate->OnFrame();
}

void RenderWidget::resizeEvent(QResizeEvent* e)
{
    QQuickWidget::resizeEvent(e);
    float32 dpi = devicePixelRatioF();
    QSize size = e->size();
    widgetDelegate->OnResized(size.width(), size.height(), dpi);
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

void RenderWidget::timerEvent(QTimerEvent* e)
{
    if (!quickWindow()->isVisible())
    {
        e->ignore();
        return;
    }

    QQuickWidget::timerEvent(e);
}

void RenderWidget::mousePressEvent(QMouseEvent* e)
{
    QQuickWidget::mousePressEvent(e);
    widgetDelegate->OnMousePressed(e);
}

void RenderWidget::mouseReleaseEvent(QMouseEvent* e)
{
    QQuickWidget::mouseReleaseEvent(e);
    widgetDelegate->OnMouseReleased(e);
}

void RenderWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
    QQuickWidget::mouseDoubleClickEvent(e);
    widgetDelegate->OnMouseDBClick(e);
}

void RenderWidget::mouseMoveEvent(QMouseEvent* e)
{
    QQuickWidget::mouseMoveEvent(e);
    widgetDelegate->OnMouseMove(e);
}

void RenderWidget::wheelEvent(QWheelEvent* e)
{
    QQuickWidget::wheelEvent(e);
    widgetDelegate->OnWheel(e);
}

void RenderWidget::keyPressEvent(QKeyEvent* e)
{
    QQuickWidget::keyPressEvent(e);
    widgetDelegate->OnKeyPressed(e);
}

void RenderWidget::keyReleaseEvent(QKeyEvent* e)
{
    QQuickWidget::keyReleaseEvent(e);
    widgetDelegate->OnKeyReleased(e);
}

bool RenderWidget::eventFilter(QObject* object, QEvent* e)
{
    if (object == quickWindow() && keyEventRecursiveGuard == false)
    {
        keyEventRecursiveGuard = true;
        SCOPE_EXIT
        {
            keyEventRecursiveGuard = false;
        };
        QEvent::Type t = e->type();
        if (t == QEvent::KeyPress || t == QEvent::KeyRelease)
        {
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

} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__