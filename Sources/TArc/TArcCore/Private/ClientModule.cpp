#include "TArcCore/ClientModule.h"

#include "Debug/DVAssert.h"

namespace tarc
{

tarc::ContextAccessor& ClientModule::GetAccessor()
{
    DVASSERT(contextAccessor != nullptr);
    return *contextAccessor;
}

void ClientModule::Init(ContextAccessor* contextAccessor_)
{
    DVASSERT(contextAccessor == nullptr);
    contextAccessor = contextAccessor_;
}

}
