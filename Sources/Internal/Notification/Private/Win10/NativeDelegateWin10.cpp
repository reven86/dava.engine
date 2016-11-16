#include "Notification/Private/Win10/NativeDelegateWin10.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/UWP/NativeServiceUWP.h"
#include "Notification/LocalNotificationController.h"

#include "Engine/Engine.h"
#include "Logger/Logger.h"
#include "Utils/UTF8Utils.h"
namespace DAVA
{
NativeDelegateImpl::NativeDelegateImpl(LocalNotificationController& controller)
    : localNotificationController(controller)
{
    Engine::Instance()->GetNativeService()->RegisterXamlApplicationListener(this);
}

NativeDelegateImpl::~NativeDelegateImpl()
{
    Engine::Instance()->GetNativeService()->UnregisterXamlApplicationListener(this);
}

void NativeDelegateImpl::OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^ launchArgs)
{
    using namespace DAVA;
    String arguments = UTF8Utils::EncodeToUTF8(launchArgs->Arguments->Data());
    if (launchArgs->Kind == Windows::ApplicationModel::Activation::ActivationKind::Launch)
    {
        Platform::String ^ launchString = launchArgs->Arguments;
        if (!arguments.empty())
        {
            String uidStr = UTF8Utils::EncodeToUTF8(launchString->Data());
            auto function = [this, arguments]()
            {
                localNotificationController.OnNotificationPressed(arguments);
            };
            Engine::Instance()->RunAsyncOnMainThread(function);
        }
    }
    Logger::Debug("TestBed.NativeDelegateWin10::OnLaunched: arguments=%s, activationKind=%d", arguments.c_str(), launchArgs->Kind);
}
}
#endif // __DAVAENGINE_WIN_UAP__
