#include "PluginManager/Plugin.h"
#include "ModuleManager/IModule.h"
#include "Reflection/ReflectionRegistrator.h"

using namespace DAVA;

int DAVAMain(Vector<String> cmdline)
{
    return 0;
}

class SamplePlugin : public DAVA::IModule
{
public:
    enum eStatus
    {
        ES_UNKNOWN,
        ES_INIT,
        ES_SHUTDOWN
    };

    SamplePlugin(DAVA::Engine* engine)
        : IModule(engine)
    {
        statusList.emplace_back(eStatus::ES_UNKNOWN);
    }

    ~SamplePlugin()
    {
    }

    void Init() override
    {
        statusList.emplace_back(eStatus::ES_INIT);
    }

    void Shutdown() override
    {
        statusList.emplace_back(eStatus::ES_SHUTDOWN);
    }

private:
    DAVA::Vector<eStatus> statusList;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SamplePlugin, IModule)
    {
    }
};

EXPORT_PLUGIN(SamplePlugin)
