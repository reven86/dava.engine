#pragma once

#include "TArcCore/ControllerModule.h"

namespace DAVA
{
namespace TArc
{
class MockControllerModule : public ControllerModule
{
public:
    MOCK_METHOD1(OnRenderSystemInitialized, void(Window* w));
    MOCK_METHOD1(CanWindowBeClosedSilently, bool(const WindowKey& key));
    MOCK_METHOD1(SaveOnWindowClose, void(const WindowKey& key));
    MOCK_METHOD1(RestoreOnWindowClose, void(const WindowKey& key));
    MOCK_METHOD1(OnContextCreated, void(DataContext& context));
    MOCK_METHOD1(OnContextDeleted, void(DataContext& context));
    MOCK_METHOD1(OnWindowClosed, void(const WindowKey& key));
    MOCK_METHOD0(PostInit, void());
}
} // namespace TArc
} // namespace DAVA
