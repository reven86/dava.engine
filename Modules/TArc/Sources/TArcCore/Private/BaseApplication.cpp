#include "TArcCore/BaseApplication.h"
#include "TArcCore/TArcCore.h"
#include "Testing/TArcTestCore.h"

#include "Engine/Engine.h"
#include "CommandLine/CommandLineParser.h"

namespace DAVA
{
namespace TArc
{
int BaseApplication::Run()
{
    Engine e;
    EngineInitInfo initInfo = GetInitInfo();

    // TODO remove this retain after merge with PR-2443
    SafeRetain(initInfo.options.Get());
    e.SetOptions(initInfo.options.Get());

    e.cleanup.Connect(this, &BaseApplication::Cleanup);

    if (CommandLineParser::CommandIsFound("--selftest"))
    {
        e.Init(eEngineRunMode::GUI_EMBEDDED, initInfo.modules);

        TestCore testCore(e);
        return e.Run();
    }
    else
    {
        e.Init(initInfo.runMode, initInfo.modules);

        Core core(e);
        CreateModules(&core);
        return e.Run();
    }
}
} // namespace TArc
} // namespace DAVA