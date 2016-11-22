#pragma once

#include "TArc/Core/ContextManager.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/Core/OperationInvoker.h"

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
