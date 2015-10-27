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


#include "ControlMapper.h"

#include "DAVAEngine.h"
#include "UI/UIControlSystem.h"
#include "Platform/Qt5/QtLayer.h"

#include <QWindow>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QDragMoveEvent>
#include <QDebug>


ControlMapper::ControlMapper( QWindow *w )
    : window(w)
{
}

void ControlMapper::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
        case Qt::Key_Alt:
            DAVA::InputSystem::Instance()->GetKeyboard().OnKeyPressed( DAVA::DVKEY_ALT );
            return;
        case Qt::Key_Control:
            DAVA::InputSystem::Instance()->GetKeyboard().OnKeyPressed( DAVA::DVKEY_CTRL );
            return;
        case Qt::Key_Shift:
            DAVA::InputSystem::Instance()->GetKeyboard().OnKeyPressed( DAVA::DVKEY_SHIFT );
            return;
        case Qt::Key_CapsLock:
            DAVA::InputSystem::Instance()->GetKeyboard().OnKeyPressed( DAVA::DVKEY_CAPSLOCK );
            return;
        case Qt::Key_Meta:
            // Ignore Win key on windows, Ctrl key on OSX
            return;
        default:
            break;
    }
    
#ifdef Q_OS_MAC
    // OS X doesn't send keyReleaseEvent for hotkeys with Cmd modifier
    // Temporary disable such keys
    if ( e->modifiers() & Qt::Modifier::CTRL )
    {
        return;
    }
#endif
    
    const auto davaKey = DAVA::InputSystem::Instance()->GetKeyboard().GetDavaKeyForSystemKey( e->nativeVirtualKey() );
    if (davaKey != DAVA::DVKEY_UNKNOWN)
    {
        DAVA::QtLayer::Instance()->KeyPressed( davaKey, e->count(), e->timestamp() );
    }
}

void ControlMapper::keyReleaseEvent(QKeyEvent *e)
{
    switch (e->key())
    {
        case Qt::Key_Alt:
            DAVA::InputSystem::Instance()->GetKeyboard().OnKeyUnpressed( DAVA::DVKEY_ALT );
            return;
        case Qt::Key_Control:
            DAVA::InputSystem::Instance()->GetKeyboard().OnKeyUnpressed( DAVA::DVKEY_CTRL );
            return;
        case Qt::Key_Shift:
            DAVA::InputSystem::Instance()->GetKeyboard().OnKeyUnpressed( DAVA::DVKEY_SHIFT );
            return;
        case Qt::Key_CapsLock:
            DAVA::InputSystem::Instance()->GetKeyboard().OnKeyUnpressed( DAVA::DVKEY_CAPSLOCK );
            return;
        case Qt::Key_Meta:
            // Ignore Win key on windows, Ctrl key on OSX
            return;
        default:
            break;
    }
    
    const auto davaKey = DAVA::InputSystem::Instance()->GetKeyboard().GetDavaKeyForSystemKey( e->nativeVirtualKey() );
    if (davaKey != DAVA::DVKEY_UNKNOWN)
    {
        DAVA::InputSystem::Instance()->GetKeyboard().OnKeyUnpressed(davaKey);
    }
}

void ControlMapper::mouseMoveEvent(QMouseEvent * event)
{
    auto davaEvent = MapMouseEventToDAVA(event->pos(), event->button(), event->timestamp());
    const auto dragWasApplied = event->buttons() != Qt::NoButton;

    davaEvent.phase = dragWasApplied ? DAVA::UIEvent::Phase::DRAG : DAVA::UIEvent::Phase::MOVE;
    davaEvent.device = DAVA::UIEvent::Device::MOUSE;

    DAVA::QtLayer::Instance()->MouseEvent( davaEvent );
}

void ControlMapper::mousePressEvent(QMouseEvent * event)
{
    auto davaEvent = MapMouseEventToDAVA(event->pos(), event->button(), event->timestamp());
    davaEvent.phase = DAVA::UIEvent::Phase::BEGAN;
    davaEvent.device = DAVA::UIEvent::Device::MOUSE;

    DAVA::QtLayer::Instance()->MouseEvent(davaEvent);
}

void ControlMapper::mouseReleaseEvent(QMouseEvent * event)
{
    auto davaEvent = MapMouseEventToDAVA(event->pos(), event->button(), event->timestamp());
    davaEvent.phase = DAVA::UIEvent::Phase::ENDED;
    davaEvent.device = DAVA::UIEvent::Device::MOUSE;

    DAVA::QtLayer::Instance()->MouseEvent(davaEvent);
}

void ControlMapper::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void ControlMapper::wheelEvent(QWheelEvent *event)
{
    const auto currentDPR = static_cast<int>( window->devicePixelRatio() );

    DAVA::UIEvent davaEvent;
    davaEvent.point = DAVA::Vector2( event->pixelDelta().x(), event->pixelDelta().y() );
    davaEvent.timestamp = 0;
    davaEvent.phase = DAVA::UIEvent::Phase::WHEEL;
    davaEvent.device = DAVA::UIEvent::Device::MOUSE;

    DAVA::QtLayer::Instance()->MouseEvent( davaEvent );
}

void ControlMapper::dragMoveEvent(QDragMoveEvent * event)
{
    DAVA::UIEvent davaEvent;
    auto pos = event->pos();
    const auto currentDPR = static_cast<int>( window->devicePixelRatio() );

    davaEvent.physPoint = DAVA::Vector2(pos.x() * currentDPR, pos.y() * currentDPR);
    davaEvent.tid = MapQtButtonToDAVA(Qt::LeftButton);
    davaEvent.timestamp = 0;
    davaEvent.tapCount = 1;
    davaEvent.phase = DAVA::UIEvent::Phase::MOVE;
    davaEvent.device = DAVA::UIEvent::Device::MOUSE;

    DAVA::QtLayer::Instance()->MouseEvent( davaEvent );
}

void ControlMapper::releaseKeyboard()
{
    DAVA::InputSystem::Instance()->GetKeyboard().ClearAllKeys();
}

DAVA::UIEvent ControlMapper::MapMouseEventToDAVA( const QPoint& pos, const Qt::MouseButton button, ulong timestamp )
{
    DAVA::UIEvent davaEvent;
    auto davaButton = MapQtButtonToDAVA( button );
    
    int currentDPR = window->devicePixelRatio();
    davaEvent.point = davaEvent.physPoint = DAVA::Vector2(pos.x() * currentDPR, pos.y() * currentDPR);
    davaEvent.tid = davaButton;
    davaEvent.timestamp = timestamp;
    davaEvent.tapCount = 1;
    
    return davaEvent;
}

DAVA::UIEvent::eButtonID ControlMapper::MapQtButtonToDAVA(const Qt::MouseButton button) const
{
    switch (button)
    {
        case Qt::LeftButton:
            return DAVA::UIEvent::BUTTON_1;
            
        case Qt::RightButton:
            return DAVA::UIEvent::BUTTON_2;
            
        case Qt::MiddleButton:
            return DAVA::UIEvent::BUTTON_3;
            
        default:
            break;
    }
    
    return DAVA::UIEvent::BUTTON_NONE;
}
