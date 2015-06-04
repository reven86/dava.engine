/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "MouseHelper.h"

#include <QMouseEvent>


MouseHelper::MouseHelper(QWidget* _w)
    : QObject(_w)
      , w(_w)
      , isHover(false)
      , isPressed(false)
      , clickDist(4)
      , dblClickDist(4)
{
    Q_ASSERT( w );
    w->installEventFilter(this);
}

MouseHelper::~MouseHelper()
{
}

bool MouseHelper::IsPressed() const
{
    return isPressed;
}

bool MouseHelper::eventFilter(QObject* obj, QEvent* e)
{
    if (obj == w)
    {
        switch (e->type())
        {
        case QEvent::Enter:
            enterEvent(e);
            break;
        case QEvent::Leave:
            leaveEvent(e);
            break;
        case QEvent::MouseMove:
            mouseMoveEvent(static_cast<QMouseEvent *>(e));
            break;
        case QEvent::MouseButtonPress:
            mousePressEvent(static_cast<QMouseEvent *>(e));
            break;
        case QEvent::MouseButtonRelease:
            mouseReleaseEvent(static_cast<QMouseEvent *>(e));
            break;
        case QEvent::Wheel:
            mouseWheelEvent(static_cast<QWheelEvent *>(e));
            break;

        default:
            break;
        }
    }

    return QObject::eventFilter(obj, e);
}

void MouseHelper::enterEvent(QEvent* event)
{
    Q_UNUSED(event);
    isHover = true;
    emit mouseEntered();
}

void MouseHelper::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);
    isHover = false;
    emit mouseLeaved();
}

void MouseHelper::mouseMoveEvent(QMouseEvent* event)
{
    pos = event->pos();
    emit mouseMove(pos);
}

void MouseHelper::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return ;

    pos = event->pos();
    clickPos = pos;
    isPressed = true;
    emit mousePress(pos);
}

void MouseHelper::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return ;
    if (!isPressed)
        return ;

    pos = event->pos();
    isPressed = false;
    emit mouseRelease(pos);

    const QPoint dist = pos - clickPos;
    const int sqrDist = dist.x() * dist.x() + dist.y() * dist.y();
    if (sqrDist <= clickDist * clickDist)
    {
        emit clicked();
    }
}

void MouseHelper::mouseWheelEvent(QWheelEvent* event)
{
    emit mouseWheel(event->delta());
}
