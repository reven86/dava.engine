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

using namespace DAVA;

namespace ControlMapperDetails
{
Vector<UIEvent::MouseButton> MapQtButtonToDAVA(const Qt::MouseButtons buttons)
{
    Vector<UIEvent::MouseButton> mouseButtons;

    static Map<Qt::MouseButton, UIEvent::MouseButton> acceptableButtons = {
        { Qt::LeftButton, UIEvent::MouseButton::LEFT },
        { Qt::RightButton, UIEvent::MouseButton::RIGHT },
        { Qt::MiddleButton, UIEvent::MouseButton::MIDDLE },
        { Qt::XButton1, UIEvent::MouseButton::EXTENDED1 },
        { Qt::XButton2, UIEvent::MouseButton::EXTENDED2 }
    };

    for (const auto& buttonsPair : acceptableButtons)
    {
        if (buttons.testFlag(buttonsPair.first))
        {
            mouseButtons.push_back(buttonsPair.second);
        }
    }
    return mouseButtons;
}
} //namespace ControlMapperDetails

ControlMapper::ControlMapper(QWindow* w)
    : window(w)
{
}

Key ConvertQtCommandKeysToDava(int qtKey)
{
    Key result = Key::UNKNOWN;
    switch (qtKey)
    {
    case Qt::Key_Shift:
        result = Key::LSHIFT;
        break;
    case Qt::Key_Control:
        result = Key::LCTRL;
        break;
    case Qt::Key_Alt:
        result = Key::LALT;
        break;
    case Qt::Key_AltGr:
        result = Key::RALT;
        break;
    default:
    {
        const int Kostil_KeyForRussianLanguage_A = 1060;
        const int Kostil_KeyForRussianLanguage_Z = 1060 + 26;
        if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z)
        {
            int key = static_cast<int>(Key::KEY_A) + (qtKey - Qt::Key_A);
            result = static_cast<Key>(key);
        }
        else if (qtKey >= Kostil_KeyForRussianLanguage_A && qtKey <= Kostil_KeyForRussianLanguage_Z)
        {
            int key = static_cast<int>(Key::KEY_A) + (qtKey - Kostil_KeyForRussianLanguage_A);
            result = static_cast<Key>(key);
        }
    }
    break;
    }
    return result;
}

void ControlMapper::keyPressEvent(QKeyEvent* e)
{
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
    const Key davaKey = DavaQtKeyboard::GetDavaKeyForSystemKey(virtKey);
#else
    qint32 virtKey = e->nativeVirtualKey();
    Key davaKey = Key::UNKNOWN;
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
    const Key davaKey = DavaQtKeyboard::GetDavaKeyForSystemKey(virtKey);
#else
    qint32 virtKey = e->nativeVirtualKey();
    Key davaKey = Key::UNKNOWN;
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
    Vector<UIEvent> mouseButtons = MapMouseEventToDAVA(event->pos(), event->buttons(), event->timestamp());

    for (UIEvent& ev : mouseButtons)
    {
        if (ev.mouseButton != UIEvent::MouseButton::NONE)
        {
            ev.phase = UIEvent::Phase::DRAG;
        }
        else
        {
            ev.phase = UIEvent::Phase::MOVE;
        }
        QtLayer::Instance()->MouseEvent(ev);
    }
}

void ControlMapper::mousePressEvent(QMouseEvent* event)
{
    Vector<UIEvent> mouseButtons = MapMouseEventToDAVA(event->pos(), event->button(), event->timestamp());

    for (UIEvent& ev : mouseButtons)
    {
        ev.phase = UIEvent::Phase::BEGAN;
        QtLayer::Instance()->MouseEvent(ev);
    }
}

void ControlMapper::mouseReleaseEvent(QMouseEvent* event)
{
    Vector<UIEvent> mouseButtons = MapMouseEventToDAVA(event->pos(), event->button(), event->timestamp());

    for (UIEvent& ev : mouseButtons)
    {
        ev.phase = UIEvent::Phase::ENDED;
        QtLayer::Instance()->MouseEvent(ev);
    }
}

void ControlMapper::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
}

void ControlMapper::wheelEvent(QWheelEvent* event)
{
    Vector<UIEvent> mouseButtons = MapMouseEventToDAVA(event->pos(), event->buttons(), event->timestamp());

    for (UIEvent& ev : mouseButtons)
    {
        ev.phase = UIEvent::Phase::WHEEL;
        ev.timestamp = 0;

        ev.wheelDelta.x = event->pixelDelta().x();
        ev.wheelDelta.y = event->pixelDelta().y();
        QtLayer::Instance()->MouseEvent(ev);
    }
}

void ControlMapper::dragMoveEvent(QDragMoveEvent* event)
{
    UIEvent davaEvent;
    QPoint pos = event->pos();
    const int currentDPR = static_cast<int>(window->devicePixelRatio());

    davaEvent.physPoint = Vector2(pos.x() * currentDPR, pos.y() * currentDPR);
    davaEvent.mouseButton = UIEvent::MouseButton::LEFT;
    davaEvent.timestamp = 0;
    davaEvent.phase = UIEvent::Phase::MOVE;
    davaEvent.device = UIEvent::Device::MOUSE;

    QtLayer::Instance()->MouseEvent(davaEvent);
}

void ControlMapper::releaseKeyboard()
{
    DavaQtKeyboard::ClearAllKeys();
}

Vector<UIEvent> ControlMapper::MapMouseEventToDAVA(const QPoint& pos, const Qt::MouseButtons buttons, ulong timestamp) const
{
    Vector<UIEvent> events;

    Vector<UIEvent::MouseButton> davaButtons = ControlMapperDetails::MapQtButtonToDAVA(buttons);
    events.reserve(davaButtons.size());

    int currentDPR = window->devicePixelRatio();

    UIEvent davaEvent;
    davaEvent.physPoint = Vector2(pos.x() * currentDPR, pos.y() * currentDPR);
    davaEvent.timestamp = timestamp;
    davaEvent.device = UIEvent::Device::MOUSE;

    if (davaButtons.empty())
    {
        davaEvent.mouseButton = UIEvent::MouseButton::NONE;
        events.push_back(davaEvent);
    }
    else
    {
        for (UIEvent::MouseButton btn : davaButtons)
        {
            davaEvent.mouseButton = btn;
            events.push_back(davaEvent);
        }
    }
    return events;
}
