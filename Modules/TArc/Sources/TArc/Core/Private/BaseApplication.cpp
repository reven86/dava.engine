#include "TArc/Core/BaseApplication.h"
#include "TArc/Core/Core.h"
#include "TArc/Testing/TArcTestCore.h"
#include "TArc/DataProcessing/TArcAnyCasts.h"
#include "TArc/Utils/AssertGuard.h"
#include "QtHelpers/RunGuard.h"

#include "Engine/Engine.h"
#include "CommandLine/CommandLineParser.h"

void InitTArcResources()
{
    Q_INIT_RESOURCE(TArcResources);
}

namespace DAVA
{
namespace TArc
{
int BaseApplication::Run()
{
    InitTArcResources();
    if (!AllowMultipleInstances())
    {
        QtHelpers::RunGuard runGuard(GetInstanceKey());
        if (runGuard.TryToRun())
        {
            return RunImpl();
        }
        else
            return 0;
    }

    return RunImpl();
}

int BaseApplication::RunImpl()
{
    Engine e;
    EngineInitInfo initInfo = GetInitInfo();

    // TODO remove this retain after merge with PR-2443
    SafeRetain(initInfo.options.Get());

    e.cleanup.Connect(this, &BaseApplication::Cleanup);

    if (CommandLineParser::CommandIsFound("--selftest"))
    {
        isTestEnv = true;

        SetupToolsAssertHandlers(eApplicationMode::TEST_MODE);
        e.Init(eEngineRunMode::GUI_EMBEDDED, initInfo.modules, initInfo.options.Get());
        RegisterAnyCasts();
        RegisterEditorAnyCasts();
        RegisterReflectionExtensions();

        const EngineContext* engineContext = e.GetContext();
        DVASSERT(engineContext);
        Init(engineContext);

        TestCore testCore(e);
        return e.Run();
    }
    else
    {
        SetupToolsAssertHandlers(initInfo.runMode == eEngineRunMode::CONSOLE_MODE ? eApplicationMode::CONSOLE_MODE : eApplicationMode::GUI_MODE);
        e.Init(initInfo.runMode, initInfo.modules, initInfo.options.Get());
        RegisterAnyCasts();
        RegisterEditorAnyCasts();
        RegisterReflectionExtensions();

        Core core(e);
        Init(&core);
        CreateModules(&core);
        return e.Run();
    }
}

void BaseApplication::Init(const EngineContext* /*engineContext*/)
{
}

void BaseApplication::Init(Core* tarcCore)
{
    DVASSERT(tarcCore != nullptr);
    Init(tarcCore->GetEngineContext());
}

void BaseApplication::RegisterEditorAnyCasts()
{
}

void BaseApplication::RegisterReflectionExtensions()
{
}

void BaseApplication::Cleanup()
{
}

bool BaseApplication::AllowMultipleInstances() const
{
    return true;
}

QString BaseApplication::GetInstanceKey() const
{
    return QString();
}

bool BaseApplication::IsTestEnvironment() const
{
    return isTestEnv;
}

} // namespace TArc
} // namespace DAVA
