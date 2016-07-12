#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#include "Engine/Private/EngineFwd.h"

namespace DAVA
{
class NativeService final
{
public:
private:
    NativeService(Private::CoreWin32* c);

private:
    Private::CoreWin32* core = nullptr;

    // Friends
    friend Private::CoreWin32;
};

} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
