#pragma once

#include "TArcCore/ClientModule.h"
#include "TArcCore/ContextManager.h"

namespace tarc
{

class ControllerModule : public ClientModule
{
protected:
    virtual void OnRenderSystemInitialized(DAVA::Window& w) = 0;
    ContextManager& GetContextManager();

private:
    void SetContextManager(ContextManager* contextManager);

private:
    friend class Core;
    ContextManager* contextManager = nullptr;
};

}
