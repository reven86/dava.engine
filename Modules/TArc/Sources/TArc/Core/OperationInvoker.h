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
    virtual void Invoke(int operationId, const Any& a) = 0;
    virtual void Invoke(int operationId, const Any& a1, const Any& a2) = 0;
    virtual void Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3) = 0;
    virtual void Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4) = 0;
    virtual void Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5) = 0;
    virtual void Invoke(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5, const Any& a6) = 0;
};
} // namespace TArc
} // namespace DAVA