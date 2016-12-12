#include "Tests/SamplePluginTest.h"
#include "ModuleManager/ModuleManager.h"

using namespace DAVA;

SamplePluginTest::SamplePluginTest(TestBed& app)
    : BaseScreen(app, "Sample plugin test")
    ,
    engine(app.GetEngine())
{
}

void SamplePluginTest::LoadResources()
{
    BaseScreen::LoadResources();

    ModuleManager& mm = *engine.GetContext()->moduleManager;
    FileSystem& ff = *engine.GetContext()->fileSystem;

    FilePath executDir = ff.GetCurrentExecutableDirectory();

    Vector<FilePath> pluginsList;
    pluginsList = mm.PluginList(executDir, ModuleManager::EFP_Auto);

    for (auto it = rbegin(pluginsList); it != rend(pluginsList); ++it)
    {
        mm.InitPlugin(*it);
    }
}

void SamplePluginTest::UnloadResources()
{
    BaseScreen::UnloadResources();

    ModuleManager& mm = *engine.GetContext()->moduleManager;
    mm.ShutdownPlugins();
}
