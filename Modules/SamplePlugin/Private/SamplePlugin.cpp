#include "PluginManager/Plugin.h"
#include "ModuleManager/IModule.h"

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
};

EXPORT_PLUGIN(SamplePlugin)
