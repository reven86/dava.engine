#if defined(__DAVAENGINE_COREV2__)

namespace DAVA
{
namespace Private
{
// Forward declarations of private Engine-related classes

class Dispatcher;
struct DispatcherEvent;

class EngineBackend;

#if defined(__DAVAENGINE_WIN32__)
class CoreWin32;
using PlatformCore = CoreWin32;

class WindowWin32;
using PlatformWindow = WindowWin32;
#else
#error "Platform is not implemented"
#endif

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
