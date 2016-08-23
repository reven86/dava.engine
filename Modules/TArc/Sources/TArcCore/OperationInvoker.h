#pragma once

#include "Base/Any.h"

namespace DAVA
{
namespace TArc
{
class OperationInvoker
{
public:
    virtual void Invoke(int operationId) = 0;
    virtual void Invoke(int operationId, const DAVA::Any& a) = 0;
    virtual void Invoke(int operationId, const DAVA::Any& a1, const DAVA::Any& a2) = 0;
    virtual void Invoke(int operationId, const DAVA::Any& a1, const DAVA::Any& a2, const DAVA::Any& a3) = 0;
    virtual void Invoke(int operationId, const DAVA::Any& a1, const DAVA::Any& a2, const DAVA::Any& a3, const DAVA::Any& a4) = 0;
    virtual void Invoke(int operationId, const DAVA::Any& a1, const DAVA::Any& a2, const DAVA::Any& a3, const DAVA::Any& a4, const DAVA::Any& a5) = 0;
    virtual void Invoke(int operationId, const DAVA::Any& a1, const DAVA::Any& a2, const DAVA::Any& a3, const DAVA::Any& a4, const DAVA::Any& a5, const DAVA::Any& a6) = 0;
};
} // namespace TArc
} // namespace DAVA