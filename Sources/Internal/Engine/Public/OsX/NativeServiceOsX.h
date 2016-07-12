#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/EngineFwd.h"

namespace DAVA
{
class NativeService final
{
public:
private:
    NativeService(Private::CoreOsX* c);

private:
    Private::CoreOsX* core = nullptr;

    // Friends
    friend Private::CoreOsX;
};

} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
