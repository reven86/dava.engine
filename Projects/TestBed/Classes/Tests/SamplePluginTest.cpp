#include "Tests/SamplePluginTest.h"
#include "Infrastructure/TestBed.h"
#include "Engine/Engine.h"

using namespace DAVA;

SamplePluginTest::SamplePluginTest(TestBed& app)
    : BaseScreen(app, "Sample plugin test")
    , engine(app.GetEngine())
    , pluginDescriptor(nullptr)
{
}

void SamplePluginTest::LoadResources()
{
    BaseScreen::LoadResources();

    PluginManager& mm = *engine.GetContext()->pluginManager;
    FileSystem& ff = *engine.GetContext()->fileSystem;

    FilePath executDir = ff.GetCurrentExecutableDirectory();

    Vector<FilePath> pluginsList;
    pluginsList = mm.GetPlugins(executDir, PluginManager::EFP_Auto);

    for (auto& path : pluginsList)
    {
        pluginDescriptor = mm.InitPlugin(path);
        DVASSERT(pluginDescriptor != nullptr);
    }
}

void SamplePluginTest::UnloadResources()
{
    BaseScreen::UnloadResources();

    PluginManager& mm = *engine.GetContext()->pluginManager;
    mm.ShutdownPlugin(pluginDescriptor);    
}
