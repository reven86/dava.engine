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
// Bridge between C++ and Objective-C for iOS's WindowBackend class
// Responsibilities:
//  - holds neccesary Objective-C objects
//  - posts events to dispatcher
//
// WindowNativeBridgeiOS is friend of iOS's WindowBackend
struct WindowNativeBridgeiOS final
{
    WindowNativeBridgeiOS(WindowBackend* wbackend);
    ~WindowNativeBridgeiOS();

    bool DoCreateWindow(float32 x, float32 y, float32 width, float32 height);
    void DoResizeWindow(float32 width, float32 height);
    void DoCloseWindow();

    void TriggerPlatformEvents();

    //////////////////////////////////////////////////////////////////////////

    WindowBackend* windowBackend = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
