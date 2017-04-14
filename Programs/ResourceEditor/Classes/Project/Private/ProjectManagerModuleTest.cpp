#include "Classes/Project/ProjectManagerModule.h"
#include "Classes/Project/ProjectManagerData.h"
#include "Classes/Application/LaunchModule.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Settings/Settings.h"
#include "Classes/Settings/SettingsManager.h"

#include "Classes/CommandLine/Private/CommandLineModuleTestUtils.h"

#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockListener.h"
#include "TArc/DataProcessing/DataWrapper.h"

#include <QMainWindow>
#include <QWidget>
#include <QMenuBar>
#include <QMenu>
#include <QTest>

#include <gmock/gmock.h>
#include "Base/Any.h"

namespace PMT
{
const DAVA::String projectModulePropertiesKey = "ProjectManagerProperties";
const DAVA::String testFolder = DAVA::String("~doc:/Test/");
const DAVA::String firstFakeProjectPath = DAVA::String("~doc:/Test/ProjectManagerTest1/");

class SceneManagerMockModule : public DAVA::TArc::ClientModule
{
protected:
    void PostInit() override
    {
        using namespace DAVA;
        using namespace TArc;

        RegisterOperation(REGlobal::CreateNewSceneOperation.ID, this, &SceneManagerMockModule::CreateNewSceneMock);
        RegisterOperation(REGlobal::CloseAllScenesOperation.ID, this, &SceneManagerMockModule::CloseAllScenesMock);

        ContextAccessor* accessor = GetAccessor();
        {
            PropertiesItem item = accessor->CreatePropertiesNode(projectModulePropertiesKey);
            lastOpenedProject = item.Get<FilePath>(Settings::Internal_LastProjectPath.c_str());
            item.Set(Settings::Internal_LastProjectPath.c_str(), FilePath());
        }
        {
            PropertiesItem item = accessor->CreatePropertiesNode(projectsHistoryKey);
            projectsHistory = item.Get<Vector<String>>(recentItemsKey);
            item.Set(recentItemsKey, Vector<String>());
        }

        reloadParticles = SettingsManager::GetValue(Settings::General_ReloadParticlesOnPojectOpening).AsBool();
        SettingsManager::SetValue(Settings::General_ReloadParticlesOnPojectOpening, VariantType(false));

        laspOpenedPathInSettings = SettingsManager::GetValue(Settings::Internal_LastProjectPath).AsFilePath();
        SettingsManager::SetValue(Settings::Internal_LastProjectPath, VariantType(FilePath()));

        // prepare test environment
        {
            CommandLineModuleTestUtils::CreateTestFolder(testFolder);
        }

        {
            CommandLineModuleTestUtils::CreateProjectInfrastructure(firstFakeProjectPath);
            DAVA::TArc::PropertiesItem propsItem = GetAccessor()->CreatePropertiesNode(PMT::projectModulePropertiesKey);
            propsItem.Set(Settings::Internal_LastProjectPath.c_str(), firstFakeProjectPath);
        }
    }

    ~SceneManagerMockModule() override
    {
        using namespace DAVA::TArc;

        ContextAccessor* accessor = GetAccessor();

        SettingsManager::SetValue(Settings::General_ReloadParticlesOnPojectOpening, DAVA::VariantType(reloadParticles));
        SettingsManager::SetValue(Settings::Internal_LastProjectPath, DAVA::VariantType(laspOpenedPathInSettings));

        DAVA::TArc::PropertiesItem propsItem = accessor->CreatePropertiesNode(projectModulePropertiesKey);
        propsItem.Set(Settings::Internal_LastProjectPath.c_str(), DAVA::Any(lastOpenedProject));

        PropertiesItem item = accessor->CreatePropertiesNode(projectsHistoryKey);
        item.Set(recentItemsKey, projectsHistory);

        CommandLineModuleTestUtils::ClearTestFolder(testFolder);
    }

    void CreateNewSceneMock()
    {
    }

    void CloseAllScenesMock(bool)
    {
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SceneManagerMockModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<SceneManagerMockModule>::Begin()
        .ConstructorByPointer()
        .End();
    }

    //recent items properties names
    const DAVA::String projectsHistoryKey = "Recent projects";
    const DAVA::String recentItemsKey = "recent items";

    DAVA::Vector<DAVA::String> projectsHistory;
    DAVA::FilePath lastOpenedProject;
    DAVA::FilePath laspOpenedPathInSettings;
    bool reloadParticles = false;
};
}

DAVA_TARC_TESTCLASS(ProjectManagerTests)
{
    DAVA::TArc::DataWrapper wrapper;
    DAVA::TArc::MockListener listener;

    DAVA_TEST (LaunchAppTest)
    {
        using namespace ::testing;
        wrapper = GetAccessor()->CreateWrapper(DAVA::ReflectedTypeDB::Get<ProjectManagerData>());
        wrapper.SetListener(&listener);
        if (wrapper.HasData() == false)
        {
            InSequence sequence;
            EXPECT_CALL(*GetMockInvoker(), Invoke(static_cast<int>(REGlobal::OpenLastProjectOperation.ID)));
            EXPECT_CALL(*GetMockInvoker(), Invoke(static_cast<int>(REGlobal::CreateNewSceneOperation.ID)));
            EXPECT_CALL(listener, OnDataChanged(_, _))
            .WillOnce(Invoke([this](const DAVA::TArc::DataWrapper& w, const DAVA::Vector<DAVA::Any>& fields)
                             {
                                 TEST_VERIFY(w.HasData());
                                 TEST_VERIFY(wrapper == w);
                                 TEST_VERIFY(fields.empty());

                                 ProjectManagerData* data = GetAccessor()->GetGlobalContext()->GetData<ProjectManagerData>();
                                 TEST_VERIFY(data != nullptr);
                                 TEST_VERIFY(data->IsOpened() == true);

                                 DAVA::FilePath path = data->GetProjectPath();

                                 TEST_VERIFY(path == PMT::firstFakeProjectPath);
                             }));
        }
        else
        {
            ProjectManagerData* data = GetAccessor()->GetGlobalContext()->GetData<ProjectManagerData>();
            TEST_VERIFY(data != nullptr);
            TEST_VERIFY(data->IsOpened() == true);

            DAVA::FilePath path = data->GetProjectPath();

            TEST_VERIFY(path == PMT::firstFakeProjectPath);
        }
    }

    DAVA_TEST (CloseProject)
    {
        using namespace ::testing;
        using namespace DAVA::TArc;

        EXPECT_CALL(*GetMockInvoker(), Invoke(REGlobal::CloseAllScenesOperation.ID, _))
        .WillOnce(Invoke([this](int id, const DAVA::Any& arg1)
                         {
                             TEST_VERIFY(arg1.CanCast<bool>());
                             TEST_VERIFY(arg1.Cast<bool>() == true);

                             DAVA::Vector<DataContext::ContextID> contexts;
                             GetAccessor()->ForEachContext([&contexts](DataContext& ctx)
                                                           {
                                                               contexts.push_back(ctx.GetID());
                                                           });

                             ContextManager* mng = GetContextManager();
                             for (DataContext::ContextID id : contexts)
                             {
                                 mng->DeleteContext(id);
                             }
                         }));

        EXPECT_CALL(listener, OnDataChanged(wrapper, _))
        .WillOnce(Invoke([this](const DataWrapper& w, const DAVA::Vector<DAVA::Any>& fields)
                         {
                             TEST_VERIFY(wrapper.HasData());
                             TEST_VERIFY(fields.size() == 1);
                             TEST_VERIFY(fields[0].CanCast<DAVA::String>());
                             TEST_VERIFY(fields[0].Cast<DAVA::String>() == ProjectManagerData::ProjectPathProperty);

                             TEST_VERIFY(GetAccessor()->GetGlobalContext()->GetData<ProjectManagerData>()->GetProjectPath().IsEmpty());
                         }));

        QWidget* wnd = GetWindow(DAVA::TArc::mainWindowKey);
        QMainWindow* mainWnd = qobject_cast<QMainWindow*>(wnd);
        TEST_VERIFY(wnd != nullptr);

        QMenuBar* menu = mainWnd->menuBar();
        QMenu* fileMenu = menu->findChild<QMenu*>(MenuItems::menuFile);

        QAction* closeProjectAction = nullptr;
        QList<QAction*> actions = fileMenu->actions();
        foreach (QAction* action, actions)
        {
            if (action->objectName() == "Close Project")
            {
                closeProjectAction = action;
                break;
            }
        }

        TEST_VERIFY(closeProjectAction != nullptr);
        closeProjectAction->triggered(false);
    }

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(PMT::SceneManagerMockModule)
    DECLARE_TESTED_MODULE(LaunchModule)
    DECLARE_TESTED_MODULE(ProjectManagerModule)
    END_TESTED_MODULES()
};
