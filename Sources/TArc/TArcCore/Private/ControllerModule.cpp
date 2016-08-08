#include "TArcCore/ControllerModule.h"

namespace tarc
{

ContextManager& ControllerModule::GetContextManager()
{
    DVASSERT(contextManager != nullptr);
    return *contextManager;
}

void ControllerModule::SetContextManager(ContextManager* contextManager_)
{
    contextManager = contextManager_;
}

}