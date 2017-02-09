#include "Modules/ProjectModule/ProjectModule.h"
#include "Modules/ProjectModule/ProjectData.h"
#include "Modules/ProjectModule/Project.h"

#include "Test/TestHelpers.h"

#include "Application/QEGlobal.h"

#include <TArc/Testing/TArcUnitTests.h>
#include <TArc/Testing/MockListener.h>
#include <TArc/DataProcessing/DataNode.h>

#include <gmock/gmock.h>

#include <QWidget>
#include <QAction>

namespace PMT
{
class DocumentsManagerMockModule;
}

DAVA_TARC_TESTCLASS(ProjectManagerTests)
{
    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(PMT::DocumentsManagerMockModule)
    DECLARE_TESTED_MODULE(ProjectModule)
    END_TESTED_MODULES()

    DAVA_TEST (OpenEmptyLastProject)
    {
        using namespace DAVA;
        using namespace TArc;
        using namespace ::testing;

        InvokeOperation(QEGlobal::OpenLastProject.ID);
        ContextAccessor* accessor = GetAccessor();
        DataContext* globalContext = accessor->GetGlobalContext();
        ProjectData* projectData = globalContext->GetData<ProjectData>();
        TEST_VERIFY(projectData == nullptr);
    }

    DAVA_TEST (CreateNewProject)
    {
        using namespace DAVA;
        using namespace TArc;
        using namespace ::testing;

        EXPECT_CALL(*GetMockInvoker(), Invoke(QEGlobal::CloseAllDocuments.ID))
        .WillOnce(Invoke([this](int id)
                         {
                             DeleteAllContexts();
                         }));

        ContextAccessor* accessor = GetAccessor();

        TestHelpers::CreateTestProjectFolder();

        String projectPath = TestHelpers::GetTestProjectPath().GetAbsolutePathname();
        InvokeOperation(ProjectModuleTesting::CreateProjectOperation.ID, QString::fromStdString(projectPath));

        DataContext* globalContext = accessor->GetGlobalContext();
        ProjectData* projectData = globalContext->GetData<ProjectData>();
        TEST_VERIFY(projectData != nullptr);
        TEST_VERIFY(projectData->GetUiDirectory().absolute.IsEmpty() == false);
        TEST_VERIFY(projectData->GetProjectDirectory().IsEmpty() == false);
    }

    DAVA_TEST (CloseProject)
    {
        using namespace DAVA;
        using namespace TArc;
        using namespace ::testing;

        CreateDummyContext();

        wrapper = GetAccessor()->CreateWrapper(DAVA::ReflectedTypeDB::Get<ProjectData>());
        wrapper.SetListener(&listener);

        EXPECT_CALL(*GetMockInvoker(), Invoke(QEGlobal::CloseAllDocuments.ID))
        .WillOnce(Invoke([this](int id)
                         {
                             DeleteAllContexts();
                         }));
        EXPECT_CALL(listener, OnDataChanged(wrapper, _))
        .WillOnce(Invoke([this](const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
                         {
                             TEST_VERIFY(wrapper.HasData() == false);
                             ProjectData* data = GetAccessor()->GetGlobalContext()->GetData<ProjectData>();
                             TEST_VERIFY(data == nullptr);
                             TEST_VERIFY(GetAccessor()->GetContextCount() == 0);
                         }));

        QAction* action = TestHelpers::FindActionInMenus(GetWindow(QEGlobal::windowKey), fileMenuName, closeProjectActionName);
        action->triggered();
    }

    DAVA_TEST (OpenLastProject)
    {
        using namespace DAVA;
        using namespace TArc;
        using namespace ::testing;

        EXPECT_CALL(listener, OnDataChanged(wrapper, _))
        .WillOnce(Invoke([this](const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
                         {
                             TEST_VERIFY(wrapper.HasData() == true);
                             ProjectData* data = GetAccessor()->GetGlobalContext()->GetData<ProjectData>();
                             TEST_VERIFY(data != nullptr);
                             TEST_VERIFY(data->GetUiDirectory().absolute.IsEmpty() == false);
                             TEST_VERIFY(data->GetProjectDirectory().IsEmpty() == false);
                         }));

        InvokeOperation(QEGlobal::OpenLastProject.ID);
    }

    DAVA_TEST (FailCloseProject)
    {
        using namespace DAVA;
        using namespace TArc;
        using namespace ::testing;

        CreateDummyContext();

        EXPECT_CALL(*GetMockInvoker(), Invoke(QEGlobal::CloseAllDocuments.ID));

        QAction* action = TestHelpers::FindActionInMenus(GetWindow(QEGlobal::windowKey), fileMenuName, closeProjectActionName);
        action->triggered();

        ContextAccessor* accessor = GetAccessor();
        DataContext* globalContext = accessor->GetGlobalContext();
        ProjectData* projectData = globalContext->GetData<ProjectData>();
        TEST_VERIFY(projectData != nullptr);
        TEST_VERIFY(projectData->GetUiDirectory().absolute.IsEmpty() == false);
        TEST_VERIFY(projectData->GetProjectDirectory().IsEmpty() == false);

        //remove context or project will never be closed
        DeleteAllContexts();
    }

    DAVA_TEST (CloseWindowTest)
    {
        using namespace ::testing;

        EXPECT_CALL(*GetMockInvoker(), Invoke(QEGlobal::CloseAllDocuments.ID));
        EXPECT_CALL(listener, OnDataChanged(wrapper, _))
        .WillOnce(Invoke([this](const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
                         {
                             TEST_VERIFY(wrapper.HasData() == false);
                             ProjectData* data = GetAccessor()->GetGlobalContext()->GetData<ProjectData>();
                             TEST_VERIFY(data == nullptr);
                             TEST_VERIFY(GetAccessor()->GetContextCount() == 0);
                         }));

        QWidget* widget = GetWindow(QEGlobal::windowKey);
        widget->close();
    }

    const QString closeProjectActionName = "Close project";
    const QString fileMenuName = "File";

    const DAVA::FilePath firstFakeProjectPath = DAVA::FilePath("~doc:/Test/ProjectManagerTest1/");

    DAVA::TArc::DataWrapper wrapper;
    DAVA::TArc::MockListener listener;

    void DeleteAllContexts()
    {
        using namespace DAVA;
        using namespace TArc;
        Vector<DataContext::ContextID> contexts;
        GetAccessor()->ForEachContext([&contexts](DataContext& ctx)
                                      {
                                          contexts.push_back(ctx.GetID());
                                      });

        ContextManager* mng = GetContextManager();
        for (DataContext::ContextID id : contexts)
        {
            mng->DeleteContext(id);
        }
    };

    void CreateDummyContext()
    {
        //create test context to make sure project will close it
        DAVA::TArc::ContextManager* manager = GetContextManager();
        DAVA::Vector<std::unique_ptr<DAVA::TArc::DataNode>> dummy;
        manager->CreateContext(std::move(dummy));
    }
};

namespace PMT
{
class DocumentsManagerMockModule : public DAVA::TArc::ClientModule
{
protected:
    void PostInit() override
    {
        using namespace DAVA;
        using namespace TArc;

        RegisterOperation(QEGlobal::CloseAllDocuments.ID, this, &DocumentsManagerMockModule::CloseAllDocumentsMock);

        ContextAccessor* accessor = GetAccessor();
        {
            PropertiesItem item = accessor->CreatePropertiesNode(projectsHistoryKey);
            projectsHistory = item.Get<Vector<String>>(recentItemsKey);
            item.Set(recentItemsKey, Vector<String>());
        }
        {
            PropertiesItem item = accessor->CreatePropertiesNode(projectModulePropertiesKey);
            lastProject = item.Get<String>(lastProjectKey);
            item.Set(lastProjectKey, String());
        }
    }

    ~DocumentsManagerMockModule() override
    {
        using namespace DAVA::TArc;
        ContextAccessor* accessor = GetAccessor();
        {
            PropertiesItem item = accessor->CreatePropertiesNode(projectsHistoryKey);
            item.Set(recentItemsKey, projectsHistory);
        }
        {
            PropertiesItem item = accessor->CreatePropertiesNode(projectModulePropertiesKey);
            item.Set(projectsHistoryKey, lastProject);
        }
        TestHelpers::ClearTestFolder();
    }

    void CloseAllDocumentsMock()
    {
    }

    //last project properties names
    const DAVA::String projectModulePropertiesKey = "ProjectModuleProperties";
    const DAVA::String lastProjectKey = "Last project";

    //recent items properties names
    const DAVA::String projectsHistoryKey = "Projects history";
    const DAVA::String recentItemsKey = "recent items";

    DAVA::Vector<DAVA::String> projectsHistory;
    DAVA::String lastProject;

    DAVA_VIRTUAL_REFLECTION(DocumentsManagerMockModule, DAVA::TArc::ClientModule);
};

DAVA_VIRTUAL_REFLECTION_IMPL(DocumentsManagerMockModule)
{
    DAVA::ReflectionRegistrator<DocumentsManagerMockModule>::Begin()
    .ConstructorByPointer()
    .End();
}
}
