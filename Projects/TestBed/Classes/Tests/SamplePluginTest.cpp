#include "Tests/SamplePluginTest.h"
#include "PluginManager/PluginManager.h"
#include "Infrastructure/TestBed.h"
#include "Engine/Engine.h"

using namespace DAVA;

SamplePluginTest::SamplePluginTest(TestBed& app)
    : BaseScreen(app, "Sample plugin test")
    ,engine(app.GetEngine())
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

    for ( auto& path : pluginsList )
    {
        mm.InitPlugin(path);
    }
}

void SamplePluginTest::UnloadResources()
{
    BaseScreen::UnloadResources();

    PluginManager& mm = *engine.GetContext()->pluginManager;
    mm.ShutdownPlugins();
}
