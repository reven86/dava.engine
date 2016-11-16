#include "TArc/Core/ControllerModule.h"
#include "TArc/Core/Private/CoreInterface.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
namespace TArc
{
bool ControllerModule::ControlWindowClosing(const WindowKey& key, QCloseEvent* event)
{
    return false;
}

ContextManager* ControllerModule::GetContextManager()
{
    DVASSERT(coreInterface != nullptr);
    return coreInterface;
}
} // namespace TArc
} // namespace DAVA