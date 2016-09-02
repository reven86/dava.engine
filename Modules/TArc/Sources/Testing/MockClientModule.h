#pragma once

#include "TArcCore/ClientModule.h"

namespace DAVA
{
namespace TArc
{

class MockClientModule: public ClientModule
{
public:
    MOCK_METHOD1(OnContextCreated, void(DataContext& context));
    MOCK_METHOD1(OnContextDeleted, void(DataContext& context));
    MOCK_METHOD1(OnWindowClosed, void(const WindowKey& key));
    MOCK_METHOD0(PostInit, void());
};

} // namespace TArc
} // namespace DAVA
