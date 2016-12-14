#include "PluginManager/PluginManager.h"
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

//

extern "C" {

    //required a function
    PLUGIN_FUNCTION_EXPORT
    DAVA::IModule* CreatPlugin(DAVA::Engine* engine)
    {
        return new SamplePlugin(engine);
    }

    PLUGIN_FUNCTION_EXPORT
    void DestroyPlugin(DAVA::IModule* plugin)
    {
        delete plugin;
    }

}