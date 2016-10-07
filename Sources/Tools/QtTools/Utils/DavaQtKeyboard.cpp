#include "DavaQtKeyboard.h"

#include "Input/InputSystem.h"

namespace DAVA
{
Key DavaQtKeyboard::GetDavaKeyForSystemKey(uint32 virtualKey)
{
    return InputSystem::Instance()->GetKeyboard().GetDavaKeyForSystemKey(virtualKey);
}
void DavaQtKeyboard::ClearAllKeys()
{
    InputSystem::Instance()->GetKeyboard().ClearAllKeys();
}
}