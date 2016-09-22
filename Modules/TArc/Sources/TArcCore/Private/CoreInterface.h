#pragma once

#include "TArcCore/ContextManager.h"
#include "TArcCore/ContextAccessor.h"
#include "TArcCore/OperationInvoker.h"

namespace DAVA
{
class AnyFn;
namespace TArc
{
class CoreInterface : public ContextAccessor, public ContextManager, public OperationInvoker
{
public:
    virtual void RegisterOperation(int operationID, AnyFn&& fn) = 0;
};
} // namespace TArc
} // namespace DAVA
