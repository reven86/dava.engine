#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_QT__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Qt/WindowBackendQt.h"

namespace DAVA
{
namespace PlatformApi
{
void AcqureWindowContext(Window* targetWindow)
{
    using namespace DAVA::Private;
    WindowBackend* wb = EngineBackend::GetWindowBackend(targetWindow);
    wb->AcqureContext();
}

void ReleaseWindowContext(Window* targetWindow)
{
    using namespace DAVA::Private;
    WindowBackend* wb = EngineBackend::GetWindowBackend(targetWindow);
    wb->ReleaseContext();
}

} // namespace PlatformApi
} // namespace DAVA

#endif // defined(__DAVAENGINE_QT__)
#endif // defined(__DAVAENGINE_COREV2__)
