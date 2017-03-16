#include "TArc/Core/ClientModule.h"
#include "TArc/Core/Private/CoreInterface.h"

#include <Debug/DVAssert.h>

namespace DAVA
{
namespace TArc
{
ContextAccessor* ClientModule::GetAccessor()
{
    DVASSERT(coreInterface != nullptr);
    return coreInterface;
}

const ContextAccessor* ClientModule::GetAccessor() const
{
    DVASSERT(coreInterface != nullptr);
    return coreInterface;
}

UI* ClientModule::GetUI()
{
    DVASSERT(ui != nullptr);
    return ui.get();
}

OperationInvoker* ClientModule::GetInvoker()
{
    DVASSERT(coreInterface != nullptr);
    return coreInterface;
}

void ClientModule::Init(CoreInterface* coreInterface_, std::unique_ptr<UI>&& ui_)
{
    DVASSERT(coreInterface == nullptr);
    DVASSERT(ui == nullptr);
    coreInterface = coreInterface_;
    ui = std::move(ui_);
}
} // namespace TArc
} // namespace DAVA
