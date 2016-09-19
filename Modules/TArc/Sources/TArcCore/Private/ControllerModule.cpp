#include "TArcCore/ControllerModule.h"
#include "TArcCore/Private/CoreInterface.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
namespace TArc
{
ContextManager& ControllerModule::GetContextManager()
{
    DVASSERT(coreInterface != nullptr);
    return *coreInterface;
}
} // namespace TArc
} // namespace DAVA