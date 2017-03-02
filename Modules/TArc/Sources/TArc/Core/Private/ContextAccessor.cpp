#include "TArc/Core/ContextAccessor.h"

const DAVA::TArc::DataContext* DAVA::TArc::ContextAccessor::GetGlobalContext() const
{
    return const_cast<ContextAccessor*>(this)->GetGlobalContext();
}

const DAVA::TArc::DataContext* DAVA::TArc::ContextAccessor::GetContext(DataContext::ContextID contextId) const
{
    return const_cast<ContextAccessor*>(this)->GetContext(contextId);
}

const DAVA::TArc::DataContext* DAVA::TArc::ContextAccessor::GetActiveContext() const
{
    return const_cast<ContextAccessor*>(this)->GetActiveContext();
}
