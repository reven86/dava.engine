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

#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

#include "UI/UIControlSystem.h"
#include "Platform/Qt5/QtLayer.h"

#include "ControlMapper.h"

PUSH_QT_WARNING_SUPRESSOR
#include <QKeyEvent>
#include <QWheelEvent>
#include <QDragMoveEvent>
#include <QDebug>
POP_QT_WARNING_SUPRESSOR

namespace DAVA
{
// we have to create this wrapper inside DAVA namespace for friend keyworkd works on private keyboard field
class DavaQtKeyboard
{
public:
    static Key GetDavaKeyForSystemKey(uint32 virtualKey)
    {
        return InputSystem::Instance()->GetKeyboard().GetDavaKeyForSystemKey(virtualKey);
    }
    static void ClearAllKeys()
    {
        InputSystem::Instance()->GetKeyboard().ClearAllKeys();
    }
};
} // end namespace DAVA

ControlMapper::ControlMapper(QWindow* w)
    : window(w)
{
}

DAVA::Key ConvertQtCommandKeysToDava(int qtKey)
{
    DAVA::Key result = DAVA::Key::UNKNOWN;
    switch (qtKey)
    {
    case Qt::Key_Shift:
        result = DAVA::Key::LSHIFT;
        break;
    case Qt::Key_Control:
        result = DAVA::Key::LCTRL;
        break;
    case Qt::Key_Alt:
        result = DAVA::Key::LALT;
        break;
    case Qt::Key_AltGr:
        result = DAVA::Key::RALT;
        break;
    case Qt::Key_Meta:
        result = DAVA::Key::LWIN;
        break;
    default:
    {
        const int Kostil_KeyForRussianLanguage_A = 1060;
        const int Kostil_KeyForRussianLanguage_Z = 1060 + 26;
        if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z)
        {
            int key = static_cast<int>(DAVA::Key::KEY_A) + (qtKey - Qt::Key_A);
            result = static_cast<DAVA::Key>(key);
        }
        else if (qtKey >= Kostil_KeyForRussianLanguage_A && qtKey <= Kostil_KeyForRussianLanguage_Z)
        {
            int key = static_cast<int>(DAVA::Key::KEY_A) + (qtKey - Kostil_KeyForRussianLanguage_A);
            result = static_cast<DAVA::Key>(key);
        }
    }
    break;
    }
    return result;
}

void ControlMapper::keyPressEvent(QKeyEvent* e)
{
    using namespace DAVA;   
#ifdef Q_OS_WIN
    uint32 nativeModif = e->nativeModifiers();
    uint32 nativeScanCode = e->nativeScanCode();
    uint32 virtKey = e->nativeVirtualKey();
    if ((1 << 24) & nativeModif)
    {
        virtKey |= 0x100;
    }
    if (VK_SHIFT == virtKey && nativeScanCode == 0x36) // is right shift key
    {
        virtKey |= 0x100;
    }
    const auto davaKey = DavaQtKeyboard::GetDavaKeyForSystemKey(virtKey);
#else
    qint32 virtKey = e->nativeVirtualKey();
    auto davaKey = Key::UNKNOWN;
    if (virtKey != 0)
    {
        davaKey = DavaQtKeyboard::GetDavaKeyForSystemKey(virtKey);
    }
    else
    {
        davaKey = ConvertQtCommandKeysToDava(e->key());
    }
#endif

    if (davaKey != Key::UNKNOWN)
    {
        QtLayer::Instance()->KeyPressed(davaKey, e->timestamp());
    }
}

void ControlMapper::keyReleaseEvent(QKeyEvent* e)
{
    using namespace DAVA;
#ifdef Q_OS_WIN
    uint32 nativeModif = e->nativeModifiers();
    uint32 nativeScanCode = e->nativeScanCode();
    uint32 virtKey = e->nativeVirtualKey();
    if ((1 << 24) & nativeModif)
    {
        virtKey |= 0x100;
    }
    if (VK_SHIFT == virtKey && nativeScanCode == 0x36) // is right shift key
    {
        virtKey |= 0x100;
    }
    const auto davaKey = DavaQtKeyboard::GetDavaKeyForSystemKey(virtKey);
#else
    qint32 virtKey = e->nativeVirtualKey();
    auto davaKey = Key::UNKNOWN;
    if (virtKey != 0)
    {
        davaKey = DavaQtKeyboard::GetDavaKeyForSystemKey(virtKey);
    }
    else
    {
        davaKey = ConvertQtCommandKeysToDava(e->key());
    }
#endif
    if (davaKey != Key::UNKNOWN)
    {
        QtLayer::Instance()->KeyReleased(davaKey, e->timestamp());
    }
}

void ControlMapper::mouseMoveEvent(QMouseEvent* event)
{
    auto& mouseButtons = MapMouseEventToDAVA(event->pos(), event->buttons(), event->timestamp());

    for (auto& ev : mouseButtons)
    {
        if (ev.mouseButton != DAVA::UIEvent::MouseButton::NONE)
        {
            ev.phase = DAVA::UIEvent::Phase::DRAG;
        }
        else
        {
            ev.phase = DAVA::UIEvent::Phase::MOVE;
        }
        DAVA::QtLayer::Instance()->MouseEvent(ev);
    }
}

void ControlMapper::mousePressEvent(QMouseEvent* event)
{
    auto& mouseButtons = MapMouseEventToDAVA(event->pos(), event->button(), event->timestamp());

    for (auto& ev : mouseButtons)
    {
        ev.phase = DAVA::UIEvent::Phase::BEGAN;
        DAVA::QtLayer::Instance()->MouseEvent(ev);
    }
}

void ControlMapper::mouseReleaseEvent(QMouseEvent* event)
{
    auto& mouseButtons = MapMouseEventToDAVA(event->pos(), event->button(), event->timestamp());

    for (auto& ev : mouseButtons)
    {
        ev.phase = DAVA::UIEvent::Phase::ENDED;
        DAVA::QtLayer::Instance()->MouseEvent(ev);
    }
}

void ControlMapper::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
}

void ControlMapper::wheelEvent(QWheelEvent* event)
{
    DAVA::UIEvent davaEvent;
    davaEvent.wheelDelta.x = event->pixelDelta().x();
    davaEvent.wheelDelta.y = event->pixelDelta().y();
    davaEvent.timestamp = 0;
    davaEvent.phase = DAVA::UIEvent::Phase::WHEEL;
    davaEvent.device = DAVA::UIEvent::Device::MOUSE;

    DAVA::QtLayer::Instance()->MouseEvent(davaEvent);
}

void ControlMapper::dragMoveEvent(QDragMoveEvent* event)
{
    DAVA::UIEvent davaEvent;
    auto pos = event->pos();
    const auto currentDPR = static_cast<int>(window->devicePixelRatio());

    davaEvent.physPoint = DAVA::Vector2(pos.x() * currentDPR, pos.y() * currentDPR);
    davaEvent.mouseButton = DAVA::UIEvent::MouseButton::LEFT;
    davaEvent.timestamp = 0;
    davaEvent.tapCount = 1;
    davaEvent.phase = DAVA::UIEvent::Phase::MOVE;
    davaEvent.device = DAVA::UIEvent::Device::MOUSE;

    DAVA::QtLayer::Instance()->MouseEvent(davaEvent);
}

void ControlMapper::releaseKeyboard()
{
    DAVA::DavaQtKeyboard::ClearAllKeys();
}

DAVA::Vector<DAVA::UIEvent>& ControlMapper::MapMouseEventToDAVA(const QPoint& pos, const Qt::MouseButtons buttons, ulong timestamp) const
{
    using namespace DAVA;
    static Vector<UIEvent> events;

    events.clear();

    auto& davaButtons = MapQtButtonToDAVA(buttons);

    int currentDPR = window->devicePixelRatio();

    UIEvent davaEvent;
    davaEvent.physPoint = Vector2(pos.x() * currentDPR, pos.y() * currentDPR);
    davaEvent.timestamp = timestamp;
    davaEvent.tapCount = 1;
    davaEvent.device = DAVA::UIEvent::Device::MOUSE;

    if (davaButtons.empty())
    {
        davaEvent.mouseButton = UIEvent::MouseButton::NONE;
        events.push_back(davaEvent);
    }
    else
    {
        for (auto btn : davaButtons)
        {
            davaEvent.mouseButton = btn;
            events.push_back(davaEvent);
        }
    }
    return events;
}

DAVA::Vector<DAVA::UIEvent::MouseButton>& ControlMapper::MapQtButtonToDAVA(const Qt::MouseButtons button)
{
    using namespace DAVA;
    static Vector<UIEvent::MouseButton> mouseButtons;

    mouseButtons.clear();

    if (button.testFlag(Qt::LeftButton))
    {
        mouseButtons.push_back(UIEvent::MouseButton::LEFT);
    }
    if (button.testFlag(Qt::RightButton))
    {
        mouseButtons.push_back(UIEvent::MouseButton::RIGHT);
    }
    if (button.testFlag(Qt::MiddleButton))
    {
        mouseButtons.push_back(UIEvent::MouseButton::MIDDLE);
    }
    if (button.testFlag(Qt::XButton1))
    {
        mouseButtons.push_back(UIEvent::MouseButton::EXTENDED1);
    }
    if (button.testFlag(Qt::XButton2))
    {
        mouseButtons.push_back(UIEvent::MouseButton::EXTENDED2);
    }

    return mouseButtons;
}
