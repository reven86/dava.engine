#include "TArcCore/ClientModule.h"

#include "Debug/DVAssert.h"

namespace DAVA
{
namespace TArc
{
ContextAccessor& ClientModule::GetAccessor()
{
    DVASSERT(contextAccessor != nullptr);
    return *contextAccessor;
}

UI& ClientModule::GetUI()
{
    DVASSERT(ui != nullptr);
    return *ui;
}

void ClientModule::Init(ContextAccessor* contextAccessor_, UI* ui_)
{
    DVASSERT(contextAccessor == nullptr);
    DVASSERT(ui == nullptr);
    contextAccessor = contextAccessor_;
    ui = ui_;
}
} // namespace TArc
} // namespace DAVA
