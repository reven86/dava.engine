#pragma once

#include "TArcCore/ClientModule.h"
#include "TArcCore/ContextManager.h"

namespace tarc
{

class ControllerModule : public ClientModule
{
public:
    ContextManager& GetContextManager();

private:
    void SetContextManager(ContextManager* contextManager);

private:
    friend class Core;
    ContextManager* contextManager = nullptr;
};

}
