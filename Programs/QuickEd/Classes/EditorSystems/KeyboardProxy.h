#pragma once

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
