#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_QT__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Qt/PlatformCoreQt.h"
#include "Engine/Private/Qt/WindowBackendQt.h"

namespace DAVA
{
namespace PlatformApi
{
namespace Qt
{
void AcquireWindowContext(Window* targetWindow)
{
    using namespace DAVA::Private;
    WindowBackend* wb = EngineBackend::GetWindowBackend(targetWindow);
    wb->AcquireContext();
}

void ReleaseWindowContext(Window* targetWindow)
{
    using namespace DAVA::Private;
    WindowBackend* wb = EngineBackend::GetWindowBackend(targetWindow);
    wb->ReleaseContext();
}

QApplication* GetApplication()
{
    using namespace DAVA::Private;
    return EngineBackend::Instance()->GetPlatformCore()->GetApplication();
}

RenderWidget* GetRenderWidget()
{
    using namespace DAVA::Private;
    return EngineBackend::Instance()->GetPlatformCore()->GetRenderWidget();
}

} // namespace Qt
} // namespace PlatformApi
} // namespace DAVA

#endif // defined(__DAVAENGINE_QT__)
#endif // defined(__DAVAENGINE_COREV2__)
