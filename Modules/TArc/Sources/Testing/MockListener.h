#pragma once

#include "DataProcessing/DataListener.h"

#include <gmock/gmock-generated-function-mockers.h>

namespace DAVA
{
namespace TArc
{
class MockListener : public DataListener
{
public:
    MOCK_METHOD2(OnDataChanged, void(const DataWrapper& wrapper, const Set<String>& fields));
};

} // namespace TArc
} // namespace DAVA
