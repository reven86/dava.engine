#include "CommandLine/SceneSaverTool.h"
#include "CommandLine/Private/REConsoleModuleTestUtils.h"

#include "Base/BaseTypes.h"


#include "Testing/TArcUnitTests.h"

namespace SSTestDetail
{
const DAVA::String projectStr = "~doc:/Test/SceneSaverTool/";
const DAVA::String scenePathnameStr = projectStr + "DataSource/3d/Scene/testScene.sc2";
const DAVA::String newProjectStr = "~doc:/Test/SceneSaverTool_new/";
const DAVA::String newScenePathnameStr = newProjectStr + "DataSource/3d/Scene/testScene.sc2";
}

DAVA_TARC_TESTCLASS(SceneSaverToolTest)
{
    DAVA_TEST (SaveSceneTest)
    {
        using namespace DAVA;

        REConsoleModuleTestUtils::CreateProjectInfrastructure(SSTestDetail::projectStr);
        REConsoleModuleTestUtils::CreateScene(SSTestDetail::scenePathnameStr);

        {
            REConsoleModuleTestUtils::CreateProjectInfrastructure(SSTestDetail::newProjectStr);
            FilePath dataSourcePath = SSTestDetail::projectStr + "DataSource/3d/";
            FilePath newDataSourcePath = SSTestDetail::newProjectStr + "DataSource/3d/";

            Vector<String> cmdLine =
            {
              "ResourceEditor",
              "-scenesaver",
              "-save",
              "-indir",
              dataSourcePath.GetAbsolutePathname(),
              "-outdir",
              newDataSourcePath.GetAbsolutePathname(),
              "-processfile",
              FilePath(SSTestDetail::scenePathnameStr).GetRelativePathname(dataSourcePath),
              "-copyconverted"
            };

            std::unique_ptr<REConsoleModuleCommon> tool = std::make_unique<SceneSaverTool>(cmdLine);
            REConsoleModuleTestUtils::ExecuteModule(tool.get());

            TEST_VERIFY(FileSystem::Instance()->Exists(SSTestDetail::newScenePathnameStr));

            REConsoleModuleTestUtils::ClearTestFolder(SSTestDetail::newProjectStr);
        }

        REConsoleModuleTestUtils::ClearTestFolder(SSTestDetail::projectStr);
    }

    DAVA_TEST (ResaveSceneTest)
    {
        using namespace DAVA;

        REConsoleModuleTestUtils::CreateProjectInfrastructure(SSTestDetail::projectStr);
        REConsoleModuleTestUtils::CreateScene(SSTestDetail::scenePathnameStr);

        FilePath dataSourcePath = SSTestDetail::projectStr + "DataSource/3d/";

        Vector<String> cmdLine =
        {
          "ResourceEditor",
          "-scenesaver",
          "-resave",
          "-indir",
          dataSourcePath.GetAbsolutePathname(),
          "-processfile",
          FilePath(SSTestDetail::scenePathnameStr).GetRelativePathname(dataSourcePath),
        };

        std::unique_ptr<REConsoleModuleCommon> tool = std::make_unique<SceneSaverTool>(cmdLine);
        REConsoleModuleTestUtils::ExecuteModule(tool.get());

        TEST_VERIFY(FileSystem::Instance()->Exists(SSTestDetail::scenePathnameStr));

        REConsoleModuleTestUtils::ClearTestFolder(SSTestDetail::projectStr);
    }

    DAVA_TEST (ResaveYamlTest)
    {
        using namespace DAVA;

        REConsoleModuleTestUtils::CreateProjectInfrastructure(SSTestDetail::projectStr);
        REConsoleModuleTestUtils::CreateScene(SSTestDetail::scenePathnameStr);

        FilePath dataSourcePath = SSTestDetail::projectStr + "DataSource/3d/";

        Vector<String> cmdLine =
        {
          "ResourceEditor",
          "-scenesaver",
          "-resave",
          "-yaml",
          "-indir",
          FilePath(SSTestDetail::projectStr).GetAbsolutePathname() + "Data/",
        };

        std::unique_ptr<REConsoleModuleCommon> tool = std::make_unique<SceneSaverTool>(cmdLine);
        REConsoleModuleTestUtils::ExecuteModule(tool.get());

        TEST_VERIFY(FileSystem::Instance()->Exists(SSTestDetail::projectStr + "quality.yaml"));

        REConsoleModuleTestUtils::ClearTestFolder(SSTestDetail::projectStr);
    }

    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(TArc)
    DECLARE_COVERED_FILES("SceneSaverTool.cpp")
    END_FILES_COVERED_BY_TESTS();
};
