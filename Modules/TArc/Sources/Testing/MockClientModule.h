#pragma once

#include "TArcCore/ClientModule.h"

#include "Debug/DVAssert.h"

namespace DAVA
{
namespace TArc
{
template <typename Tag>
class MockClientModule : public ClientModule
{
public:
    MockClientModule()
    {
        DVASSERT(instance == nullptr);
        instance = this;

        EXPECT_CALL(*this, PostInit());
    }

    ~MockClientModule()
    {
        DVASSERT(instance != nullptr);
        instance = nullptr;
    }

    static MockClientModule<Tag>* instance;

    MOCK_METHOD1(OnContextCreated, void(DataContext& context));
    MOCK_METHOD1(OnContextDeleted, void(DataContext& context));
    MOCK_METHOD1(OnWindowClosed, void(const WindowKey& key));
    MOCK_METHOD0(PostInit, void());
};

template <typename Tag>
MockClientModule<Tag>* MockClientModule<Tag>::instance = nullptr;

} // namespace TArc
} // namespace DAVA
