#include "TArcCore/ControllerModule.h"
#include "TArcCore/Private/CoreInterface.h"
#include "Debug/DVAssert.h"

namespace tarc
{

ContextManager& ControllerModule::GetContextManager()
{
    DVASSERT(coreInterface != nullptr);
    return *coreInterface;
}

}