#pragma once

#include <Base/BaseTypes.h>

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/UWP/XamlApplicationListener.h"

namespace DAVA
{
class LocalNotificationController;

struct NativeDelegateImpl : public XamlApplicationListener
{
    NativeDelegateImpl(LocalNotificationController& controller);
    virtual ~NativeDelegateImpl();
    void OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^ launchArgs) override;

private:
    LocalNotificationController& localNotificationController;
};
}
#endif // __DAVAENGINE_WIN_UAP__
