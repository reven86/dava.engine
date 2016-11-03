#include "TArc/Core/BaseApplication.h"
#include "TArc/Core/Core.h"
#include "TArc/Testing/TArcTestCore.h"
#include "QtHelpers/RunGuard.h"

#include "Engine/Engine.h"
#include "CommandLine/CommandLineParser.h"

namespace DAVA
{
namespace TArc
{
int BaseApplication::Run()
{
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
        e.Init(eEngineRunMode::GUI_EMBEDDED, initInfo.modules, initInfo.options.Get());

        EngineContext* engineContext = e.GetContext();
        DVASSERT(engineContext);
        Init(engineContext);

        TestCore testCore(e);
        return e.Run();
    }
    else
    {
        e.Init(initInfo.runMode, initInfo.modules, initInfo.options.Get());

        Core core(e);
        Init(&core);
        CreateModules(&core);
        return e.Run();
    }
}

void BaseApplication::Init(EngineContext* /*engineContext*/)
{
}

void BaseApplication::Init(Core* tarcCore)
{
    DVASSERT(tarcCore != nullptr);
    Init(tarcCore->GetEngineContext());
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

} // namespace TArc
} // namespace DAVA
