#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)

#include "Functional/Signal.h"
#include "Engine/Private/EnginePrivateFwd.h"

#include <QWidget>
#include <QtEvents>

class QQuickWindow;
class QNativeGestureEvent;

namespace DAVA
{
class RenderWidget final : public QWidget
{
    Q_OBJECT
public:
    class IWindowDelegate
    {
    public:
        virtual void OnCreated() = 0;
        virtual bool OnUserCloseRequest() = 0;
        virtual void OnDestroyed() = 0;
        virtual void OnFrame() = 0;
        virtual void OnResized(uint32 width, uint32 height, bool isFullScreen) = 0;
        virtual void OnDpiChanged(float32 dpi) = 0;
        virtual void OnVisibilityChanged(bool isVisible) = 0;

        virtual void OnMousePressed(QMouseEvent* e) = 0;
        virtual void OnMouseReleased(QMouseEvent* e) = 0;
        virtual void OnMouseMove(QMouseEvent* e) = 0;
        virtual void OnDragMoved(QDragMoveEvent* e) = 0;
        virtual void OnMouseDBClick(QMouseEvent* e) = 0;
        virtual void OnWheel(QWheelEvent* e) = 0;
        virtual void OnNativeGesture(QNativeGestureEvent* e) = 0;

        virtual void OnKeyPressed(QKeyEvent* e) = 0;
        virtual void OnKeyReleased(QKeyEvent* e) = 0;
    };

    class IClientDelegate
    {
    public:
        virtual void OnMousePressed(QMouseEvent* e)
        {
        }
        virtual void OnMouseReleased(QMouseEvent* e)
        {
        }
        virtual void OnMouseMove(QMouseEvent* e)
        {
        }
        virtual void OnMouseDBClick(QMouseEvent* e)
        {
        }
        virtual void OnWheel(QWheelEvent* e)
        {
        }
        virtual void OnNativeGesture(QNativeGestureEvent* e)
        {
        }

        virtual void OnKeyPressed(QKeyEvent* e)
        {
        }
        virtual void OnKeyReleased(QKeyEvent* e)
        {
        }

        virtual void OnDragEntered(QDragEnterEvent* e)
        {
        }
        virtual void OnDragMoved(QDragMoveEvent* e)
        {
        }
        virtual void OnDragLeaved(QDragLeaveEvent* e)
        {
        }
        virtual void OnDrop(QDropEvent* e)
        {
        }
    };

    void SetClientDelegate(IClientDelegate* delegate);
    Signal<uint32, uint32> resized;

private:
    RenderWidget(IWindowDelegate* widgetDelegate, uint32 width, uint32 height);
    ~RenderWidget();

    bool IsInitialized() const;
    QQuickWindow* GetQQuickWindow();

private:
    friend class Private::WindowBackend;
    class RenderWidgetImpl;
    RenderWidgetImpl* impl = nullptr;
};

} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
