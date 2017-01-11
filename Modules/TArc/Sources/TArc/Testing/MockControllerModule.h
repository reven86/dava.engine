#pragma once

#include "TArc/Core/ControllerModule.h"

namespace DAVA
{
namespace TArc
{
class MockControllerModule : public ControllerModule
{
public:
    MOCK_METHOD1(OnRenderSystemInitialized, void(Window* w));
    MOCK_METHOD2(CanWindowBeClosedSilently, bool(const WindowKey& key, String& requestWindowText));
    MOCK_METHOD1(SaveOnWindowClose, void(const WindowKey& key));
    MOCK_METHOD1(RestoreOnWindowClose, void(const WindowKey& key));
    MOCK_METHOD1(OnContextCreated, void(DataContext* context));
    MOCK_METHOD1(OnContextDeleted, void(DataContext* context));
    MOCK_METHOD1(OnWindowClosed, void(const WindowKey& key));
    MOCK_METHOD0(PostInit, void());

    DAVA_VIRTUAL_REFLECTION(MockControllerModule, ControllerModule)
    {
        ReflectionRegistrator<MockControllerModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};
} // namespace TArc
} // namespace DAVA
