#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN_UAP__)
#if defined(__DAVAENGINE_COREV2__)

#include "Engine/UWP/XamlApplicationListener.h"

namespace DAVA
{
class LocalNotificationController;

namespace Private
{
struct NativeListener : public XamlApplicationListener
{
    NativeListener(LocalNotificationController& controller);
    virtual ~NativeListener();
    void OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^ launchArgs) override;

private:
    LocalNotificationController& localNotificationController;
};
} // namespace Private
} // namespace DAVA
#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_WIN_UAP__
