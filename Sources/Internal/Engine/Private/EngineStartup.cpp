#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"
#include "Engine/Private/EngineBackend.h"

extern int GameMain(DAVA::Vector<DAVA::String> cmdline);

// clang-format off

namespace DAVA
{
namespace Private
{

#if defined(__DAVAENGINE_QT__) || \
    defined(__DAVAENGINE_MACOS__) || \
    defined(__DAVAENGINE_IPHONE__) || \
    defined(__DAVAENGINE_WIN32__)

int EngineStart(const Vector<String>& cmdargs)
{
    // TODO: unique_ptr
    EngineBackend* engineBackend = new EngineBackend(cmdargs);
    int returnCode = GameMain(cmdargs);
    delete engineBackend;
    return returnCode;
}

#elif defined(__DAVAENGINE_WIN_UAP__)

extern int StartUWPApplication(const Vector<String>& cmdargs);

int EngineStart(const Vector<String>& cmdargs)
{
    return StartUWPApplication(cmdargs);
}

#endif

} // namespace Private
} // namespace DAVA

// clang-format on

#endif // __DAVAENGINE_COREV2__
