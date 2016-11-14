#include "CommandLine/DumpTool.h"
#include "CommandLine/Private/CommandLineModuleTestUtils.h"
#include "CommandLine/Private/CommandLineModuleTestExecute.h"

#include "Base/ScopedPtr.h"
#include "FileSystem/FileSystem.h"


#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/Vegetation/VegetationRenderObject.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"

#include "Testing/TArcUnitTests.h"

#include <memory>

namespace DTestDetail
{
const DAVA::String projectStr = "~doc:/Test/DumpTool/";
const DAVA::String scenePathnameStr = projectStr + "DataSource/3d/Scene/testScene.sc2";
const DAVA::String linksStr = "~doc:/Test/DumpTool/links.txt";

DAVA::Set<DAVA::String> ReadLinks()
{
    using namespace DAVA;

    Set<String> dumpedLinks;
    ScopedPtr<File> linksFile(File::Create(linksStr, File::READ | File::OPEN));
    if (linksFile)
    {
        do
        {
            String link = linksFile->ReadLine();
            if (!link.empty())
            {
                dumpedLinks.insert(link);
            }

        } while (!linksFile->IsEof());
    }
    return dumpedLinks;
}
}

DAVA_TARC_TESTCLASS(DumpToolTest)
{
    void TestLinks()
    {
        using namespace DAVA;

        Set<String> dumpedLinks = DTestDetail::ReadLinks();
        TEST_VERIFY(dumpedLinks.empty() == false);

        ScopedPtr<Scene> scene(new Scene());
        TEST_VERIFY(scene->LoadScene(DTestDetail::scenePathnameStr) == DAVA::SceneFileV2::eError::ERROR_NO_ERROR);

        uint32 entityCount = scene->GetChildrenCount();
        for (uint32 e = 0; e < entityCount; ++e)
        {
            Entity* child = scene->GetChild(e);

            //test reference to owner
            String ownerName = child->GetName().c_str() + String(".sc2");

            auto it = std::find_if(dumpedLinks.begin(), dumpedLinks.end(), [&ownerName](const String& str) {
                return str.find(ownerName) != String::npos;
            });
            TEST_VERIFY(it != dumpedLinks.end());

            RenderObject* ro = GetRenderObject(child);
            if (ro != nullptr)
            {
                auto testMaterial = [&dumpedLinks](NMaterial* mat)
                {
                    if (mat != nullptr)
                    {
                        const HashMap<FastName, MaterialTextureInfo*>& textures = mat->GetLocalTextures();
                        for (auto& tx : textures)
                        {
                            if (tx.first != FastName("heightmap"))
                            {
                                TEST_VERIFY(dumpedLinks.count(tx.second->path.GetAbsolutePathname()) == 1);
                            }
                        }
                    }
                };

                if (ro->GetType() == RenderObject::TYPE_LANDSCAPE)
                {
                    Landscape* landscape = static_cast<Landscape*>(ro);
                    testMaterial(landscape->GetMaterial());

                    TEST_VERIFY(dumpedLinks.count(landscape->GetHeightmapPathname().GetAbsolutePathname()) == 1);
                }
                else if (ro->GetType() == RenderObject::TYPE_VEGETATION)
                {
                    VegetationRenderObject* vegetation = static_cast<VegetationRenderObject*>(ro);
                    TEST_VERIFY(dumpedLinks.count(vegetation->GetCustomGeometryPath().GetAbsolutePathname()) == 1);
                    TEST_VERIFY(dumpedLinks.count(vegetation->GetLightmapPath().GetAbsolutePathname()) == 1);
                }
                else
                {
                    uint32 rbCount = ro->GetRenderBatchCount();
                    for (uint32 r = 0; r < rbCount; ++r)
                    {
                        RenderBatch* rb = ro->GetRenderBatch(r);
                        TEST_VERIFY(rb != nullptr);
                        if (rb)
                        {
                            testMaterial(rb->GetMaterial());
                        }
                    }
                }
            }

            //test particle
        }
    }

    DAVA_TEST (DumpFile)
    {
        using namespace DAVA;

        std::unique_ptr<CommandLineModuleTestUtils::TextureLoadingGuard> guard = CommandLineModuleTestUtils::CreateTextureGuard({ eGPUFamily::GPU_ORIGIN });
        CommandLineModuleTestUtils::CreateProjectInfrastructure(DTestDetail::projectStr);
        CommandLineModuleTestUtils::SceneBuilder::CreateFullScene(DTestDetail::scenePathnameStr);

        Vector<String> cmdLine =
        {
          "ResourceEditor",
          "-dump",
          "-links",
          "-indir",
          FilePath(DTestDetail::scenePathnameStr).GetDirectory().GetAbsolutePathname(),
          "-processfile",
          FilePath(DTestDetail::scenePathnameStr).GetFilename(),
          "-outfile",
          FilePath(DTestDetail::linksStr).GetAbsolutePathname()
        };

        std::unique_ptr<CommandLineModule> tool = std::make_unique<DumpTool>(cmdLine);
        CommandLineModuleTestExecute::ExecuteModule(tool.get());

        TestLinks();

        CommandLineModuleTestUtils::ClearTestFolder(DTestDetail::projectStr);
    }

    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(TArc)
    DECLARE_COVERED_FILES("DumpTool.cpp")
    END_FILES_COVERED_BY_TESTS();
};
