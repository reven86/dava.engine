#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineFwd.h"

namespace DAVA
{
class NativeService final
{
public:
private:
    NativeService(Private::CoreWinUWP* c);

private:
    Private::CoreWinUWP* core = nullptr;

    // Friends
    friend Private::CoreWinUWP;
};

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
