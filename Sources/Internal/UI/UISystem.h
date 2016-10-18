#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class UIControl;
class UIComponent;

class UISystem
{
public:
    virtual ~UISystem() = default;

    virtual void RegisterControl(UIControl* control){};
    virtual void UnregisterControl(UIControl* control){};
    virtual void RegisterComponent(UIControl* control, UIComponent* component){};
    virtual void UnregisterComponent(UIControl* control, UIComponent* component){};

    virtual void OnControlVisible(UIControl* control){};
    virtual void OnControlInvisible(UIControl* control){};

    virtual void Process(DAVA::float32 elapsedTime){};
};
}
