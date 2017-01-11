#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/PlatformApi.h"

namespace DAVA
{
class LocalNotificationController;

namespace Private
{
struct LocalNotificationListener : public PlatformApi::Win10::XamlApplicationListener
{
    LocalNotificationListener(LocalNotificationController& controller);
    virtual ~LocalNotificationListener() override;
    void OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^ launchArgs) override;

private:
    LocalNotificationController& localNotificationController;
};
} // namespace Private
} // namespace DAVA
#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
