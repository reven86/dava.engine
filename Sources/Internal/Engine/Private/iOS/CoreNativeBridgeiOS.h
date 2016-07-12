#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
namespace Private
{
// Bridge between C++ and Objective-C for iOS's PlatformCore class
// Responsibilities:
//  - holds neccesary Objective-C objects
//
// CoreNativeBridgeiOS is friend of iOS's PlatformCore
struct CoreNativeBridgeiOS final
{
    CoreNativeBridgeiOS(PlatformCore* c);
    ~CoreNativeBridgeiOS();

    void Quit();

    PlatformCore* core = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
