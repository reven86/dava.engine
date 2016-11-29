#include "Infrastructure/NativeDelegateWin10.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include <Logger/Logger.h>
#include <Utils/Utils.h>

DAVA::String ActivationKindToString(::Windows::ApplicationModel::Activation::ApplicationExecutionState execState)
{
    using ::Windows::ApplicationModel::Activation::ApplicationExecutionState;
    switch (execState)
    {
    case ApplicationExecutionState::NotRunning:
        return "not running";
    case ApplicationExecutionState::Running:
        return "running";
    case ApplicationExecutionState::Suspended:
        return "suspended";
    case ApplicationExecutionState::Terminated:
        return "terminated";
    case ApplicationExecutionState::ClosedByUser:
        return "closed by user";
    default:
        return "unknown";
    }
}

void NativeDelegateWin10::OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^ args)
{
    using namespace DAVA;
    String arguments = RTStringToString(args->Arguments);
    Logger::Debug("TestBed.NativeDelegateWin10::OnLaunched: arguments=%s, activationKind=%d", arguments.c_str(), args->Kind);
}

void NativeDelegateWin10::OnActivated(::Windows::ApplicationModel::Activation::IActivatedEventArgs ^ args)
{
    using namespace DAVA;
    Logger::Debug("TestBed.NativeDelegateWin10::OnActivated: activationKind=%d, previousExecutionState=%s", args->Kind, ActivationKindToString(args->PreviousExecutionState).c_str());
}

#endif // __DAVAENGINE_WIN_UAP__
