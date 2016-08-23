#pragma once

#include "TArcCore/ClientModule.h"

namespace DAVA
{
class Window;
}

namespace tarc
{

class ContextManager;
class ControllerModule : public ClientModule
{
protected:
    virtual void OnRenderSystemInitialized(DAVA::Window& w) = 0;
    ContextManager& GetContextManager();

    friend class Core;
};

}
