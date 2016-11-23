#pragma once

#include <Base/BaseTypes.h>

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/UWP/XamlApplicationListener.h"

namespace DAVA
{
class LocalNotificationController;

struct NativeDelegate : public XamlApplicationListener
{
    NativeDelegate(LocalNotificationController& controller);
    virtual ~NativeDelegate();
    void OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^ launchArgs) override;

private:
    LocalNotificationController& localNotificationController;
};
}
#endif // __DAVAENGINE_WIN_UAP__
