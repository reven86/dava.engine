#include "CommandLine/StaticOcclusionTool.h"
#include "CommandLine/Private/REConsoleModuleTestUtils.h"

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"

#include "TArc/Testing/TArcUnitTests.h"

namespace SOTestDetail
{
const DAVA::String projectStr = "~doc:/Test/StaticOcclusionTool/";
const DAVA::String scenePathnameStr = projectStr + "DataSource/3d/Scene/testScene.sc2";
}

DAVA_TARC_TESTCLASS(StaticOcclusionToolTest)
{
    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(TArc)
    DECLARE_COVERED_FILES("StaticOcclusionTool.cpp")
    END_FILES_COVERED_BY_TESTS();

    DAVA::uint32 CountSODataComponents(const DAVA::FilePath& scenePathname)
    {
        using namespace DAVA;

        ScopedPtr<Scene> scene(new Scene());
        TEST_VERIFY(scene->LoadScene(SOTestDetail::scenePathnameStr) == DAVA::SceneFileV2::eError::ERROR_NO_ERROR);

        uint32 staticOcclusionDataComponentsCount = 0;
        uint32 entityCount = scene->GetChildrenCount();
        for (uint32 e = 0; e < entityCount; ++e)
        {
            Entity* child = scene->GetChild(e);
            if (HasComponent(child, Component::STATIC_OCCLUSION_DATA_COMPONENT))
            {
                ++staticOcclusionDataComponentsCount;
            }
        }

        return staticOcclusionDataComponentsCount;
    }

    DAVA_TEST (BuildOcclusion)
    {
        using namespace DAVA;

        std::unique_ptr<REConsoleModuleTestUtils::TextureLoadingGuard> guard = REConsoleModuleTestUtils::CreateTextureGuard({ eGPUFamily::GPU_ORIGIN });
        REConsoleModuleTestUtils::CreateProjectInfrastructure(SOTestDetail::projectStr);
        REConsoleModuleTestUtils::CreateScene(SOTestDetail::scenePathnameStr);

        Vector<String> cmdLine =
        {
          "ResourceEditor",
          "-staticocclusion",
          "-build",
          "-processfile",
          FilePath(SOTestDetail::scenePathnameStr).GetAbsolutePathname()
        };

        TEST_VERIFY(CountSODataComponents(SOTestDetail::scenePathnameStr) == 0);

        std::unique_ptr<REConsoleModuleCommon> tool = std::make_unique<StaticOcclusionTool>(cmdLine);
        REConsoleModuleTestUtils::ExecuteModule(tool.get());

        TEST_VERIFY(CountSODataComponents(SOTestDetail::scenePathnameStr) == 1);

        REConsoleModuleTestUtils::ClearTestFolder(SOTestDetail::projectStr);
    }
}
;
