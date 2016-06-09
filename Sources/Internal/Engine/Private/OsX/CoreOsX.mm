#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/CoreOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

namespace DAVA
{
namespace Private
{
CoreOsX::CoreOsX() = default;

CoreOsX::~CoreOsX() = default;

Vector<String> CoreOsX::GetCommandLine(int argc, char* argv[])
{
    return Vector<String>();
}

void CoreOsX::Init()
{
}

void CoreOsX::Run()
{
}

void CoreOsX::Quit()
{
}

WindowOsX* CoreOsX::CreateNativeWindow(WindowBackend* w)
{
    return nullptr;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
