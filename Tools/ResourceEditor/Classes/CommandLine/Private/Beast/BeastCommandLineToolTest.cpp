#include "CommandLine/BeastCommandLineTool.h"

#if defined(__DAVAENGINE_BEAST__)

#include "CommandLine/Private/REConsoleModuleTestUtils.h"

#include "Base/BaseTypes.h"


#include "Testing/TArcUnitTests.h"

namespace BCLTestDetail
{
const DAVA::String projectStr = "~doc:/Test/BeastCommandLineTool/";
const DAVA::String scenePathnameStr = projectStr + "DataSource/3d/Scene/testScene.sc2";
const DAVA::String outPathnameStr = projectStr + "DataSource/3d/Scene/lightmap/";
}

DAVA_TARC_TESTCLASS(BeastCommandLineToolTest)
{
    void TestScene()
    {
        ScopedPtr<Scene> scene(new Scene());
        TEST_VERIFY(scene->LoadScene(BCLTestDetail::scenePathnameStr) == DAVA::SceneFileV2::eError::ERROR_NO_ERROR);

        uint32 entityCount = scene->GetChildrenCount();
        for (uint32 e = 0; e < entityCount; ++e)
        {
            Entity* child = scene->GetChild(e);

            RenderObject* ro = GetRenderObject(child);
            if (ro != nullptr)
            {
                NMaterial* material = nullptr;
                if (ro->GetType() == RenderObject::TYPE_LANDSCAPE)
                {
                    Landscape* landscape = static_cast<Landscape*>(ro);
                    material = landscape->GetMaterial();
                }
                else
                {
                    uint32 rbCount = ro->GetRenderBatchCount();
                    TEST_VERIFY(rbCount == 1);

                    for (uint32 r = 0; r < rbCount; ++r)
                    {
                        RenderBatch* rb = ro->GetRenderBatch(r);
                        TEST_VERIFY(rb != nullptr);
                        if (rb)
                        {
                            material = rb->GetMaterial();
                        }
                    }
                }

                //test material
                if (material != nullptr)
                {
                    const HashMap<FastName, MaterialTextureInfo*>& textures = material->GetLocalTextures();
                    for (auto& tx : textures)
                    {
                        if (tx.first == FastName("lightmap"))
                        {
                            TEST_VERIFY(FileSystem::Instance()->Exists(tx.second->path));
                            TEST_VERIFY(tx.second->path.GetDirectory().GetAbsolutePathname() == FilePath(BCLTestDetail::outPathnameStr).GetAbsolutePathname));
                        }
                    }
                }
            }
        }
        TEST_VERIFY(FileSystem::Instance()->Exists(BCLTestDetail::outPathnameStr + "landscape.png"));
    }

    DAVA_TEST (BeastTest)
    {
        using namespace DAVA;

        REConsoleModuleTestUtils::TextureLoadingGuard guard = REConsoleModuleTestUtils::CreateLoadingGuard({ eGPUFamily::GPU_ORIGIN });
        REConsoleModuleTestUtils::CreateProjectInfrastructure(BCLTestDetail::projectStr);
        REConsoleModuleTestUtils::CreateScene(BCLTestDetail::scenePathnameStr);

        Vector<String> cmdLine =
        {
          "ResourceEditor",
          "-beast",
          "-file",
          FilePath(BCLTestDetail::scenePathnameStr).GetAbsolutePathname(),
          "-output",
          FilePath(BCLTestDetail::outPathnameStr).GetAbsolutePathname(),
        };

        std::unique_ptr<REConsoleModuleCommon> tool = std::make_unique<BeastCommandLineTool>(cmdLine);
        REConsoleModuleTestUtils::ExecuteModule(tool.get());

        TestScene();

        REConsoleModuleTestUtils::ClearTestFolder(BCLTestDetail::projectStr);
    }

    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(TArc)
    DECLARE_COVERED_FILES("BeastCommandLineTool.cpp")
    END_FILES_COVERED_BY_TESTS();
}

#endif //BEAST
