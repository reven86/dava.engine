#include "TArcCore/ClientModule.h"
#include "TArcCore/Private/CoreInterface.h"

#include "Debug/DVAssert.h"

namespace DAVA
{
namespace TArc
{
ContextAccessor& ClientModule::GetAccessor()
{
    DVASSERT(coreInterface != nullptr);
    return *coreInterface;
}

UI& ClientModule::GetUI()
{
    DVASSERT(ui != nullptr);
    return *ui;
}

void ClientModule::Init(CoreInterface* coreInterface_, UI* ui_)
{
    DVASSERT(coreInterface == nullptr);
    DVASSERT(ui == nullptr);
    coreInterface = coreInterface_;
    ui = ui_;
}
} // namespace TArc
} // namespace DAVA
