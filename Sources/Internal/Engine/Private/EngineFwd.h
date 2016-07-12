// Utility header with forward declarations of Engine-related public and private classes

namespace DAVA
{
class Engine;
class Window;
class EngineContext;
class NativeService;
class WindowNativeService;

namespace Private
{
template <typename T>
class DispatcherT;
struct DispatcherEvent;
struct PlatformEvent;

using Dispatcher = DispatcherT<DispatcherEvent>;
using PlatformDispatcher = DispatcherT<PlatformEvent>;

class EngineBackend;

#if defined(__DAVAENGINE_QT__)
class CoreQt;
using PlatformCore = CoreQt;

class WindowQt;
using NativeWindow = WindowQt;
#elif defined(__DAVAENGINE_WIN32__)
class CoreWin32;
using PlatformCore = CoreWin32;

class WindowWin32;
using NativeWindow = WindowWin32;
#elif defined(__DAVAENGINE_WIN_UAP__)
class CoreWinUWP;
using PlatformCore = CoreWinUWP;

class WindowWinUWP;
using NativeWindow = WindowWinUWP;

ref struct WindowWinUWPBridge;
#elif defined(__DAVAENGINE_MACOS__)
class CoreOsX;
using PlatformCore = CoreOsX;

class WindowOsX;
using NativeWindow = WindowOsX;

struct CoreOsXObjcBridge;
struct WindowOsXObjcBridge;
#else
#if defined(__DAVAENGINE_COREV2__)
// Do not emit error when building with old core implementation 
#error "Platform is not implemented"
#endif
#endif

} // namespace Private
} // namespace DAVA
