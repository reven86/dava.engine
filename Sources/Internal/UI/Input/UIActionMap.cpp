#include "UIActionMap.h"

namespace DAVA
{
const FastName UIDefaultActions::LEFT("Left");
const FastName UIDefaultActions::RIGHT("Right");
const FastName UIDefaultActions::UP("Up");
const FastName UIDefaultActions::DOWN("Down");

const FastName UIDefaultActions::FOCUS_NEXT("FocusNext");
const FastName UIDefaultActions::FOCUS_PREV("FocusPrev");

const FastName UIDefaultActions::ACTIVATE("Activate");

const FastName UIDefaultActions::ESCAPE("Escape");

UIActionMap::UIActionMap()
{
}

UIActionMap::~UIActionMap()
{
}

void UIActionMap::Put(const FastName& name, const Action& action)
{
    actions[name] = action;
}

void UIActionMap::Remove(const FastName& name)
{
    auto it = actions.find(name);
    if (it != actions.end())
    {
        actions.erase(it);
    }
}

bool UIActionMap::Perform(const FastName& name)
{
    auto it = actions.find(name);
    if (it != actions.end())
    {
        it->second();
        return true;
    }

    return false;
}
}
