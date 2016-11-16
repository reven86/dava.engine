#pragma once

#include "TArc/DataProcessing/DataListener.h"

#include <gmock/gmock-generated-function-mockers.h>

namespace DAVA
{
namespace TArc
{
class MockListener : public DataListener
{
public:
    MOCK_METHOD2(OnDataChanged, void(const DataWrapper& wrapper, const Vector<Any>& fields));
};

} // namespace TArc
} // namespace DAVA
