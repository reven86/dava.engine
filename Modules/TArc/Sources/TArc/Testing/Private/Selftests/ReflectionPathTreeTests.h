#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"
#include "TArc/DataProcessing/PropertiesHolder.h"

#include "TArc/Controls/PropertyPanel/Private/ReflectionPathTree.h"

#include <Engine/Engine.h>
#include <FileSystem/FileSystem.h>
#include <Base/FastName.h>

// clang-format off
DAVA_TARC_TESTCLASS(ReflectionPathTreeTests)
{
    DAVA_TEST (CreationTest)
    {
        using namespace DAVA;
        using namespace DAVA::TArc;

        ReflectionPathTree tree(FastName("Root"));
        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("Child1")) == false);
        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("Child2")) == false);

        tree.AddLeaf(List<FastName>{ FastName("Child1"), FastName("Leaf1") });
        tree.AddLeaf(List<FastName>{ FastName("Child2"), FastName("Leaf2") });

        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("Child1")) == true);
        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("Child2")) == true);
        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("Leaf1")) == false);
        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("Leaf2")) == false);

        TEST_VERIFY(tree.PushRoot(FastName("Child1")) == true);
        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("Child1")) == false);
        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("Child2")) == false);
        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("Leaf1")) == true);
        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("Leaf2")) == false);
        tree.PopRoot();

        TEST_VERIFY(tree.PushRoot(FastName("Child2")) == true);
        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("Child1")) == false);
        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("Child2")) == false);
        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("Leaf1")) == false);
        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("Leaf2")) == true);
        tree.PopRoot();
    }

    DAVA_TEST (RemoveTest)
    {
        using namespace DAVA;
        using namespace DAVA::TArc;

        ReflectionPathTree tree(FastName("Root"));
        tree.AddLeaf(List<FastName>{ FastName("Child1"),
                                     FastName("SubLeaf"),
                                     FastName("Leaf") });

        tree.AddLeaf(List<FastName>{ FastName("Child1"),
                                     FastName("SubLeaf"),
                                     FastName("SecondLeaf") });

        tree.AddLeaf(List<FastName>{ FastName("Child2"),
                                     FastName("SubLeaf"),
                                     FastName("Leaf1"),
                                     FastName("Leaf2") });

        ///////////////////////////////////////////////////////////

        tree.RemoveLeaf(List<FastName>{ FastName("Child1"),
                                        FastName("SubLeaf"),
                                        FastName("Leaf") });

        TEST_VERIFY(tree.PushRoot(FastName("Child1")) == true);
        TEST_VERIFY(tree.PushRoot(FastName("SubLeaf")) == true);
        TEST_VERIFY(tree.PushRoot(FastName("Leaf")) == false);
        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("Leaf")) == false);
        tree.PopRoot();
        tree.AddLeaf(List<FastName>{ FastName("Leaf") });
        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("SubLeaf")) == true);
        TEST_VERIFY(tree.HasChildInCurrentRoot(FastName("Leaf")) == true);
        tree.PopRoot();

        tree.RemoveLeaf(List<FastName>{ FastName("Child1"),
                                        FastName("SubLeaf") });

        tree.RemoveLeaf(List<FastName>{ FastName("Child1"),
                                        FastName("SubLeaf"),
                                        FastName("Leaf") });

        tree.RemoveLeaf(List<FastName>{ FastName("Child1"),
                                        FastName("SubLeaf"),
                                        FastName("SecondLeaf") });

        TEST_VERIFY(tree.PushRoot(FastName("Child2")) == true);
        tree.RemoveLeaf(List<FastName>{ FastName("SubLeaf"), FastName("Leaf1") });
        TEST_VERIFY(tree.PushRoot(FastName("SubLeaf")) == true);
        TEST_VERIFY(tree.PushRoot(FastName("Leaf1")) == false);
        TEST_VERIFY(tree.PushRoot(FastName("Leaf2")) == false);
        tree.PopRoot();
        tree.PopRoot();

        tree.RemoveLeaf(List<FastName>{ FastName("Child2"), FastName("SubLeaf") });
        TEST_VERIFY(tree.PushRoot(FastName("Child2")) == true);
        TEST_VERIFY(tree.PushRoot(FastName("SubLeaf")) == false);
        tree.PopRoot();
        tree.AddLeaf(List<FastName>{ FastName("Child2"), FastName("SubLeaf") });
        TEST_VERIFY(tree.PushRoot(FastName("Child2")) == true);
        TEST_VERIFY(tree.PushRoot(FastName("SubLeaf")) == true);
        TEST_VERIFY(tree.PushRoot(FastName("Leaf1")) == false);
        TEST_VERIFY(tree.PushRoot(FastName("Leaf2")) == false);
        tree.AddLeaf(List<FastName>{ FastName("Leaf1") });
        TEST_VERIFY(tree.PushRoot(FastName("Leaf1")) == true);
        TEST_VERIFY(tree.PushRoot(FastName("Leaf2")) == true);

        tree.PopRoot();
        tree.PopRoot();
        tree.PopRoot();
        tree.PopRoot();
    }

    DAVA_TEST (SerializationTest)
    {
        using namespace DAVA;
        using namespace DAVA::TArc;

        ReflectionPathTree tree(FastName("Root"));
        tree.AddLeaf(List<FastName>{ FastName("Child1"),
                                     FastName("SubLeaf"),
                                     FastName("Leaf") });

        tree.AddLeaf(List<FastName>{ FastName("Child1"),
                                     FastName("SubLeaf"),
                                     FastName("SecondLeaf") });

        tree.AddLeaf(List<FastName>{ FastName("Child2"),
                                     FastName("SubLeaf"),
                                     FastName("Leaf1"),
                                     FastName("Leaf2") });

        tree.RemoveLeaf(List<FastName>{ FastName("Child2"),
                                        FastName("SubLeaf") });

        const EngineContext* ctx = GetEngineContext();
        FilePath testDir = ctx->fileSystem->GetTempDirectoryPath() + "/SerializationTest/";
        ctx->fileSystem->CreateDirectory(testDir, true);
        {
            PropertiesHolder holder("SerializationTest", testDir);
            {
                PropertiesItem treeItem = holder.CreateSubHolder("Root");
                tree.Save(treeItem);
            }
            holder.SaveToFile();
        }

        ReflectionPathTree loadedTree(FastName("OtherRoot"));
        {
            PropertiesHolder holder("SerializationTest", testDir);
            {
                PropertiesItem treeItem = holder.CreateSubHolder("Root");
                loadedTree.Load(treeItem);
            }
        }

        TEST_VERIFY(loadedTree.PushRoot(FastName("Child1")) == true);
        TEST_VERIFY(loadedTree.PushRoot(FastName("SubLeaf")) == true);
        TEST_VERIFY(loadedTree.PushRoot(FastName("Leaf")) == true);
        loadedTree.PopRoot(); // Leaf -> SubLeaf
        TEST_VERIFY(loadedTree.PushRoot(FastName("SecondLeaf")) == true);
        loadedTree.PopRoot(); // SecondLeaf -> SubLeaf
        loadedTree.PopRoot(); // SubLeaf -> Child1
        loadedTree.PopRoot(); // Child1 -> Root

        TEST_VERIFY(loadedTree.PushRoot(FastName("Child2")) == true);
        TEST_VERIFY(loadedTree.PushRoot(FastName("SubLeaf")) == false);
        loadedTree.AddLeaf(List<FastName>{ FastName("SubLeaf") });
        TEST_VERIFY(loadedTree.PushRoot(FastName("SubLeaf")) == true);
        TEST_VERIFY(loadedTree.PushRoot(FastName("Leaf1")) == true);
        TEST_VERIFY(loadedTree.PushRoot(FastName("Leaf2")) == true);
        loadedTree.PopRoot(); // Leaf2 -> Leaf1
        loadedTree.PopRoot(); // Leaf1 -> SubLeaf
        loadedTree.PopRoot(); // SubLeaf -> Child2
        loadedTree.PopRoot(); // Child2 -> Root

        ctx->fileSystem->DeleteDirectory(testDir, true);
    }
};
// clang-format on