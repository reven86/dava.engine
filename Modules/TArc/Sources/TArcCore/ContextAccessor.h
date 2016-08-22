#pragma once

#include "Engine/Public/EngineContext.h"

#include "DataProcessing/DataContext.h"
#include "DataProcessing/DataWrapper.h"

#include "Functional/Function.h"

namespace tarc
{

class ContextAccessor
{
public:
    virtual ~ContextAccessor() {}

    virtual void ForEachContext(const DAVA::Function<void(DataContext&)>& functor) = 0;

    virtual DataContext& GetGlobalContext() = 0;
    // throw std::runtime_error if context with contextID doesn't exist
    virtual DataContext& GetContext(DataContext::ContextID contextId) = 0;
    // throw std::runtime_error if there is no active context
    virtual DataContext& GetActiveContext() = 0;
    virtual bool HasActiveContext() const = 0;

    virtual DataWrapper CreateWrapper(const DAVA::ReflectedType* type) = 0;
    virtual DataWrapper CreateWrapper(const DataWrapper::DataAccessor& accessor) = 0;

    virtual DAVA::EngineContext& GetEngineContext() = 0;
};

}