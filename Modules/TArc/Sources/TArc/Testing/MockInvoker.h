#pragma once

#include "TArc/Core/OperationInvoker.h"

#include <gmock/gmock.h>

namespace DAVA
{
namespace TArc
{
class MockInvoker : public OperationInvoker
{
public:
    MOCK_METHOD1(Invoke, void(int operationId));
    MOCK_METHOD2(Invoke, void(int operationId, const Any& a));
    MOCK_METHOD3(Invoke, void(int operationId, const Any& a1, const Any& a2));
    MOCK_METHOD4(Invoke, void(int operationId, const Any& a1, const Any& a2, const Any& a3));
    MOCK_METHOD5(Invoke, void(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4));
    MOCK_METHOD6(Invoke, void(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5));
    MOCK_METHOD7(Invoke, void(int operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5, const Any& a6));
};
} // namespace TArc
} // namespace DAVA