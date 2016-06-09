// Utility header with forward declarations of Engine-related public and private classes

namespace DAVA
{
class Engine;
class Window;
class AppContext;

namespace Private
{

class Dispatcher;
struct DispatcherEvent;

class EngineBackend;
class WindowBackend;

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
#elif defined(__DAVAENGINE_MACOS__)
class CoreOsX;
using PlatformCore = CoreOsX;

class WindowOsX;
using NativeWindow = WindowOsX;
#else
#error "Platform is not implemented"
#endif

} // namespace Private
} // namespace DAVA
