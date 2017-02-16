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
    .Field(ActiveContextFieldName, &ContextAccessor::GetActiveContext, &ContextAccessor::SetActiveContext)
    .End();
}
} // namespace TArc
} // namespace DAVA