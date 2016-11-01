#pragma once

#include <Base/BaseTypes.h>

#if defined(__DAVAENGINE_WIN_UAP__)

#include <Engine/UWP/XamlApplicationListener.h>

struct NativeDelegateWin10 : public DAVA::XamlApplicationListener
{
    void OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^ launchArgs) override;
};

#endif // __DAVAENGINE_WIN_UAP__
