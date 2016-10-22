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
    virtual void OnRenderSystemInitialized(Window* w) = 0;
    virtual bool CanWindowBeClosedSilently(const WindowKey& key) = 0;
    virtual void SaveOnWindowClose(const WindowKey& key) = 0;
    virtual void RestoreOnWindowClose(const WindowKey& key) = 0;

    ContextManager& GetContextManager();

    friend class Core;
};
} // namespace TArc
} // namespace DAVA
