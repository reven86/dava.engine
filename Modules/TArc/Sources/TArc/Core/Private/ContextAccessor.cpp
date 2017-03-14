#include "TArc/Core/ContextAccessor.h"
#include "TArc/Utils/CommonFieldNames.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
namespace TArc
{
DAVA_REFLECTION_IMPL(ContextAccessor)
{
    ReflectionRegistrator<ContextAccessor>::Begin()
    .Field(ContextsFieldName, &ContextAccessor::GetContexts, nullptr)
    .Field(ActiveContextFieldName, static_cast<DataContext* (ContextAccessor::*)()>(&ContextAccessor::GetActiveContext), &ContextAccessor::SetActiveContext)
    .End();
}

const DataContext* ContextAccessor::GetGlobalContext() const
{
    return const_cast<ContextAccessor*>(this)->GetGlobalContext();
}

const DataContext* ContextAccessor::GetContext(DataContext::ContextID contextId) const
{
    return const_cast<ContextAccessor*>(this)->GetContext(contextId);
}

const DataContext* ContextAccessor::GetActiveContext() const
{
    return const_cast<ContextAccessor*>(this)->GetActiveContext();
}

} // namespace TArc
} // namespace DAVA
