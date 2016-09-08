#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Private/EnginePrivateFwd.h"
#include <QQuickWidget>

namespace DAVA
{
class RenderWidget final : public QQuickWidget
{
    Q_OBJECT
public:
    class WindowDelegate
    {
    public:
        virtual void OnCreated() = 0;
        virtual void OnDestroyed() = 0;
        virtual void OnFrame() = 0;
        virtual void OnResized(uint32 width, uint32 height, float32 dpi) = 0;
        virtual void OnVisibilityChanged(bool isVisible) = 0;

        virtual void OnMousePressed(QMouseEvent* e) = 0;
        virtual void OnMouseReleased(QMouseEvent* e) = 0;
        virtual void OnMouseMove(QMouseEvent* e) = 0;
        virtual void OnMouseDBClick(QMouseEvent* e) = 0;
        virtual void OnWheel(QWheelEvent* e) = 0;

        virtual void OnKeyPressed(QKeyEvent* e) = 0;
        virtual void OnKeyReleased(QKeyEvent* e) = 0;
    };

    class ClientDelegate
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
        virtual void OnNativeGuesture(QNativeGestureEvent* e)
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

    Q_SIGNAL void Resized(uint32 width, uint32 height);

    void SetClientDelegate(ClientDelegate* delegate);

protected:
    bool eventFilter(QObject* object, QEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;
    void timerEvent(QTimerEvent* e) override;
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dragMoveEvent(QDragMoveEvent* e) override;
    void dragLeaveEvent(QDragLeaveEvent* e) override;
    void dropEvent(QDropEvent* e) override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;

    bool event(QEvent* e) override;

private:
    RenderWidget(WindowDelegate* widgetDelegate, uint32 width, uint32 height);
    ~RenderWidget();

    Q_SLOT void OnFrame();
    Q_SLOT void OnActiveFocusItemChanged();
    Q_SLOT void OnClientDelegateDestroyed();

private:
    bool initialized = false;
    WindowDelegate* widgetDelegate = nullptr;
    ClientDelegate* clientDelegate = nullptr;
    bool keyEventRecursiveGuard = false;

    friend class Private::WindowBackend;
};

} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
