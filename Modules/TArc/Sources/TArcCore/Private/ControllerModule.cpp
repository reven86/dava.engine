#include "TArcCore/ControllerModule.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
namespace TArc
{
ContextManager& ControllerModule::GetContextManager()
{
    DVASSERT(contextManager != nullptr);
    return *contextManager;
}

void ControllerModule::SetContextManager(ContextManager* contextManager_)
{
    DVASSERT(contextManager == nullptr);
    contextManager = contextManager_;
}
} // namespace TArc
} // namespace DAVA