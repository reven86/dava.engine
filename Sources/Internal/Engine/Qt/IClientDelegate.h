#pragma once

#include <QtEvents>

namespace DAVA
{
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
} // namespace DAVA