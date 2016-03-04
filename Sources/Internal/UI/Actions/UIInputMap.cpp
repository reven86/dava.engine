#include "UIInputMap.h"

namespace DAVA
{
const FastName UIInputMap::INVALID_ACTION = FastName();

UIInputMap::UIInputMap()
{
}

UIInputMap::~UIInputMap()
{
}

void UIInputMap::BindAction(const KeyboardShortcut& shortcut, const FastName& action)
{
    inputMap[shortcut] = action;
}

const FastName& UIInputMap::FindAction(const KeyboardShortcut& shortcut) const
{
    auto it = inputMap.find(shortcut);
    if (it != inputMap.end())
    {
        return it->second;
    }

    return INVALID_ACTION;
}

void UIInputMap::RemoveShortcut(const KeyboardShortcut& shortcut)
{
    auto it = inputMap.find(shortcut);
    if (it != inputMap.end())
    {
        inputMap.erase(it);
    }
}

void UIInputMap::RemoveAction(const FastName& action)
{
    auto it = inputMap.begin();
    while (it != inputMap.end())
    {
        if (it->second == action)
        {
            it = inputMap.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void UIInputMap::Clear()
{
    inputMap.clear();
}
}
