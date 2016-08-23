#pragma once

#include "TArcCore/ContextManager.h"
#include "TArcCore/ContextAccessor.h"
#include "TArcCore/OperationInvoker.h"

namespace DAVA
{
class AnyFn;
}

namespace tarc
{

class CoreInterface: public ContextAccessor, public ContextManager, public OperationInvoker
{
public:
    virtual void RegisterOperation(int operationID, DAVA::AnyFn&& fn) = 0;
};

}