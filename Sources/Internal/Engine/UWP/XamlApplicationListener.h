#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN_UAP__)

namespace DAVA
{
/**
    \ingroup engine_win10
    Interface definition for a callbacks to be invoked when `Windows::UI::Xaml::Application` lifecycle event occurs (OnLaunched, etc).
    Only subset of `Windows::UI::Xaml::Application` methods are mapped to the interface definition, other methods are mapped as required.

    To receive callbacks application should declare class derived from `XamlApplicationListener`, implement necessary methods
    and register it through NativeService::RegisterXamlApplicationListener.
*/
struct XamlApplicationListener
{
    virtual void OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^ launchArgs)
    {
    }
};

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
