#pragma once

#include "TArc/Core/ClientModule.h"
#include "Reflection/Reflection.h"

class QCloseEvent;

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
    virtual bool ControlWindowClosing(const WindowKey& key, QCloseEvent* event);
    virtual void SaveOnWindowClose(const WindowKey& key) = 0;
    virtual void RestoreOnWindowClose(const WindowKey& key) = 0;

    ContextManager* GetContextManager();

    friend class Core;

private:
    DAVA_VIRTUAL_REFLECTION(ControllerModule, ClientModule)
    {
    }
};
} // namespace TArc
} // namespace DAVA
