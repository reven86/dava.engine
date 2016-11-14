#include "Infrastructure/NativeDelegateWin10.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include <Logger/Logger.h>
#include <Utils/Utils.h>

void NativeDelegateWin10::OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^ launchArgs)
{
    using namespace DAVA;
    String arguments = RTStringToString(launchArgs->Arguments);
    Logger::Debug("TestBed.NativeDelegateWin10::OnLaunched: arguments=%s, activationKind=%d", arguments.c_str(), launchArgs->Kind);
}

#endif // __DAVAENGINE_WIN_UAP__
