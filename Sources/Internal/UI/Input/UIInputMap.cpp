#include "UIInputMap.h"

namespace DAVA
{
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

FastName UIInputMap::FindAction(const KeyboardShortcut& shortcut) const
{
    auto it = inputMap.find(shortcut);
    if (it != inputMap.end())
    {
        return it->second;
    }

    return FastName();
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
