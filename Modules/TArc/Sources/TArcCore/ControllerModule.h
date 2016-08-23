#pragma once

#include "TArcCore/ClientModule.h"


namespace DAVA
{
class Window;
namespace TArc
{
class ContextManager;
class ControllerModule : public ClientModule
{
protected:
    virtual void OnRenderSystemInitialized(Window& w) = 0;
    ContextManager& GetContextManager();

    friend class Core;
};
} // namespace TArc
} // namespace DAVA
