#ifndef __QUICKED_KEYBOARD_PROXY_H__
#define __QUICKED_KEYBOARD_PROXY_H__

#include <Qt>

namespace KeyboardProxy
{
enum eKeys
{
    KEY_SHIFT = Qt::ShiftModifier,
    KEY_CTRL = Qt::ControlModifier,
    KEY_ALT = Qt::AltModifier
};

bool IsKeyPressed(eKeys key);
};

#endif // __QUICKED_KEYBOARD_PROXY_H__
