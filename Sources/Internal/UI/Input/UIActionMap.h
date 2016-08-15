#ifndef __DAVAENGINE_UI_ACTION_MAP_H__
#define __DAVAENGINE_UI_ACTION_MAP_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Math/Vector.h"
#include "Functional/Function.h"

namespace DAVA
{
class UIActionMap final
{
public:
    typedef DAVA::Function<void()> Action;

    UIActionMap();
    ~UIActionMap();

    void Put(const FastName& name, const Action& action);
    void Remove(const FastName& name);
    bool Perform(const FastName& name);

private:
    UnorderedMap<FastName, Action> actions;
};
}

#endif //__DAVAENGINE_UI_ACTION_MAP_H__
