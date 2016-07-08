#ifndef __DAVAENGINE_UI_INPUT_MAP_H__
#define __DAVAENGINE_UI_INPUT_MAP_H__

#include "Base/BaseTypes.h"
#include "Input/KeyboardShortcut.h"

namespace DAVA
{
class UIInputMap final
{
public:
    UIInputMap();
    ~UIInputMap();

    void BindAction(const KeyboardShortcut& shortcut, const FastName& action);
    FastName FindAction(const KeyboardShortcut& shortcut) const;

    void RemoveShortcut(const KeyboardShortcut& shortcut);
    void RemoveAction(const FastName& action);
    void Clear();

private:
    UnorderedMap<KeyboardShortcut, FastName> inputMap;
};
}


#endif //__DAVAENGINE_UI_ACTION_MAP_H__
