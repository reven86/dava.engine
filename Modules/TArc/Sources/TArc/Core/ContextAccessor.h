#pragma once

#include "Engine/EngineContext.h"

#include "TArc/DataProcessing/DataContext.h"
#include "TArc/DataProcessing/DataWrapper.h"
#include "TArc/DataProcessing/PropertiesHolder.h"

#include "Functional/Function.h"

namespace DAVA
{
namespace TArc
{
class ContextAccessor
{
public:
    virtual ~ContextAccessor()
    {
    }

    virtual void ForEachContext(const Function<void(DataContext&)>& functor) = 0;

    // never returns nullptr
    virtual DataContext* GetGlobalContext() = 0;
    // returns nullptr if there is no context with contextId
    virtual DataContext* GetContext(DataContext::ContextID contextId) = 0;
    // returns nullptr if there is no active context
    virtual DataContext* GetActiveContext() = 0;

    virtual DataWrapper CreateWrapper(const ReflectedType* type) = 0;
    virtual DataWrapper CreateWrapper(const DataWrapper::DataAccessor& accessor) = 0;
    virtual PropertiesItem CreatePropertiesNode(const String& nodeName) = 0;

    virtual EngineContext* GetEngineContext() = 0;
};
} // namespace TArc
} // namespace DAVA