#include "KeyboardProxy.h"

#include <QApplication>

bool KeyboardProxy::IsKeyPressed(eKeys key)
{
    Qt::KeyboardModifier modifier = static_cast<Qt::KeyboardModifier>(key);
    return QApplication::keyboardModifiers().testFlag(modifier);
}
