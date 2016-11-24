#pragma once

#include "TArc/Core/ClientModule.h"

#include "Debug/DVAssert.h"

#include <gmock/gmock.h>

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
        using namespace ::testing;
        DVASSERT(instance == nullptr);
        instance = this;

        EXPECT_CALL(*this, PostInit());

        ON_CALL(*this, OnContextCreated(_))
        .WillByDefault(Invoke(this, &MockClientModule::OnContextCreatedImpl));
        ON_CALL(*this, OnContextDeleted(_))
        .WillByDefault(Invoke(this, &MockClientModule::OnContextDeletedImpl));
        ON_CALL(*this, OnWindowClosed(_))
        .WillByDefault(Invoke(this, &MockClientModule::OnWindowClosedImpl));
        ON_CALL(*this, OnContextWillChanged(_, _))
        .WillByDefault(Invoke(this, &MockClientModule::OnContextWillChangedImpl));
        ON_CALL(*this, OnContextDidChanged(_, _))
        .WillByDefault(Invoke(this, &MockClientModule::OnContextDidChangedImpl));
        ON_CALL(*this, PostInit())
        .WillByDefault(Invoke(this, &MockClientModule::PostInitImpl));
    }

    ~MockClientModule()
    {
        DVASSERT(instance != nullptr);
        instance = nullptr;
    }

    static MockClientModule<Tag>* instance;

    MOCK_METHOD1(OnContextCreated, void(DataContext* context));
    MOCK_METHOD1(OnContextDeleted, void(DataContext* context));
    MOCK_METHOD1(OnWindowClosed, void(const WindowKey& key));
    MOCK_METHOD2(OnContextWillChanged, void(DataContext* current, DataContext* newOne));
    MOCK_METHOD2(OnContextDidChanged, void(DataContext* current, DataContext* oldOne));
    MOCK_METHOD0(PostInit, void());

    virtual void OnContextCreatedImpl(DataContext* context)
    {
    }
    virtual void OnContextDeletedImpl(DataContext* context)
    {
    }
    virtual void OnWindowClosedImpl(const WindowKey& key)
    {
    }
    virtual void OnContextWillChangedImpl(DataContext* current, DataContext* newOne)
    {
    }
    virtual void OnContextDidChangedImpl(DataContext* current, DataContext* oldOne)
    {
    }
    virtual void PostInitImpl()
    {
    }
};

template <typename Tag>
MockClientModule<Tag>* MockClientModule<Tag>::instance = nullptr;

} // namespace TArc
} // namespace DAVA
