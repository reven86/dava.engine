#pragma once

#include "TArc/Core/ControllerModule.h"
#include "TArc/Testing/MockDefine.h"

namespace DAVA
{
namespace TArc
{
class MockControllerModule : public ControllerModule
{
public:
    MOCK_METHOD1_VIRTUAL(OnRenderSystemInitialized, void(Window* w))
    MOCK_METHOD2_VIRTUAL(CanWindowBeClosedSilently, bool(const WindowKey& key, String& requestWindowText))
    MOCK_METHOD1_VIRTUAL(SaveOnWindowClose, void(const WindowKey& key))
    MOCK_METHOD1_VIRTUAL(RestoreOnWindowClose, void(const WindowKey& key))
    MOCK_METHOD1_VIRTUAL(OnContextCreated, void(DataContext* context))
    MOCK_METHOD1_VIRTUAL(OnContextDeleted, void(DataContext* context))
    MOCK_METHOD1_VIRTUAL(OnWindowClosed, void(const WindowKey& key))
    MOCK_METHOD0_VIRTUAL(PostInit, void())

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(MockControllerModule, ControllerModule)
    {
        ReflectionRegistrator<MockControllerModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};
} // namespace TArc
} // namespace DAVA
