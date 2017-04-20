#include "TArc/Testing/TArcTestClass.h"

#include "Modules/ProjectModule/ProjectModule.h"
#include "Modules/ProjectModule/ProjectData.h"
#include "Modules/DocumentsModule/DocumentData.h"

#include "Model/QuickEdPackageBuilder.h"
#include "Model/YamlPackageSerializer.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

#include "Utils/PackageListenerProxy.h"

#include "Test/Private/TestHelpers.h"
#include "Test/Private/ProjectSettingsGuard.h"
#include "Test/Private/MockDocumentsModule.h"

#include "Application/QEGlobal.h"

#include <TArc/Testing/TArcUnitTests.h>
#include <TArc/Testing/MockListener.h>
#include <TArc/Core/ContextManager.h>
#include <TArc/Utils/QtConnections.h>

#include <FileSystem/FileSystem.h>
#include <FileSystem/FilePath.h>
#include <FileSystem/File.h>

#include <UI/UIPackageLoader.h>
#include <UI/UIPackage.h>

#include <gmock/gmock.h>

namespace FilesTestDetails
{
class LocalMockModule;
}

DAVA_TARC_TESTCLASS(FilesTest)
{
    BEGIN_TESTED_MODULES();
    DECLARE_TESTED_MODULE(TestHelpers::ProjectSettingsGuard);
    DECLARE_TESTED_MODULE(TestHelpers::MockDocumentsModule);
    DECLARE_TESTED_MODULE(FilesTestDetails::LocalMockModule)
    DECLARE_TESTED_MODULE(ProjectModule);
    END_TESTED_MODULES();

    DAVA_TEST (TestEquality)
    {
        using namespace DAVA;
        using namespace TArc;
        using namespace TestHelpers;

        ContextAccessor* accessor = GetAccessor();
        TEST_VERIFY(accessor->GetContextCount() == 0);

        DAVA::FilePath projectPath = TestHelpers::GetTestPath() + "FilesTest";

        CreateProjectFolder(projectPath);

        String projectPathStr = projectPath.GetAbsolutePathname();
        InvokeOperation(ProjectModuleTesting::CreateProjectOperation.ID, QString::fromStdString(projectPathStr));

        DataContext* globalContext = accessor->GetGlobalContext();
        ProjectData* projectData = globalContext->GetData<ProjectData>();
        TEST_VERIFY(projectData != nullptr);
        TEST_VERIFY(projectData->GetUiDirectory().absolute.IsEmpty() == false);
        TEST_VERIFY(projectData->GetProjectDirectory().IsEmpty() == false);
        FilePath path("~res:/QuickEd/Test/testEquality.yaml");

        InvokeOperation(QEGlobal::OpenDocumentByPath.ID, path, GetContextManager());
        TEST_VERIFY(accessor->GetContextCount() == 1);

        DataContext* activeContext = accessor->GetActiveContext();
        TEST_VERIFY(activeContext != nullptr);

        DocumentData* documentData = activeContext->GetData<DocumentData>();
        PackageNode* package = documentData->GetPackageNode();

        FilePath newPath = projectPath + "/DataSource/testEquality.yaml";

        YamlPackageSerializer serializer;
        serializer.SerializePackage(package);
        TEST_VERIFY(serializer.WriteToFile(newPath));

        Vector<uint8> originalData;
        Vector<uint8> newData;
        FileSystem* fs = accessor->GetEngineContext()->fileSystem;
        fs->ReadFileContents(path, originalData);
        fs->ReadFileContents(newPath, newData);
        TEST_VERIFY(originalData == newData);
    }
};

namespace FilesTestDetails
{
class LocalMockModule : public DAVA::TArc::ClientModule
{
protected:
    void PostInit() override
    {
        using namespace DAVA;
        using namespace DAVA::TArc;
        RegisterOperation(QEGlobal::OpenDocumentByPath.ID, this, &LocalMockModule::OpenDocument);
    }

    void OpenDocument(const DAVA::FilePath& path, DAVA::TArc::ContextManager* contextManager)
    {
        using namespace DAVA;
        using namespace DAVA::TArc;

        ContextAccessor* accessor = GetAccessor();
        FileSystem* fs = accessor->GetEngineContext()->fileSystem;
        TEST_VERIFY(fs->Exists(path))

        QuickEdPackageBuilder builder;
        UIPackageLoader packageLoader;
        TEST_VERIFY(packageLoader.LoadPackage(path, &builder));

        RefPtr<PackageNode> package = builder.BuildPackage();
        TEST_VERIFY(package != nullptr);
        DAVA::Vector<std::unique_ptr<DAVA::TArc::DataNode>> initialData;
        initialData.emplace_back(new DocumentData(package));

        DataContext::ContextID id = contextManager->CreateContext(std::move(initialData));
        contextManager->ActivateContext(id);
    }

    DAVA_VIRTUAL_REFLECTION(LocalMockModule, DAVA::TArc::ClientModule);
};

DAVA_VIRTUAL_REFLECTION_IMPL(LocalMockModule)
{
    DAVA::ReflectionRegistrator<LocalMockModule>::Begin()
    .ConstructorByPointer()
    .End();
}
} //namespace SSIT
