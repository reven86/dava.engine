#pragma once

#include "Input/KeyboardDevice.h"

namespace DAVA
{
// we have to create this wrapper inside DAVA namespace for friend keyworkd works on private keyboard field
class DavaQtKeyboard
{
public:
    static Key GetDavaKeyForSystemKey(uint32 virtualKey);
    static void ClearAllKeys();
};
} // end namespace DAVA