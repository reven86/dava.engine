#include "Classes/Project/ProjectManagerModule.h"
#include "Classes/Project/ProjectManagerData.h"
#include "Classes/Application/LaunchModule.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Qt/Settings/Settings.h"
#include "Classes/Qt/Settings/SettingsManager.h"

#include "Classes/CommandLine/Private/REConsoleModuleTestUtils.h"

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
class SceneManagerMockModule : public DAVA::TArc::ClientModule
{
protected:
    void PostInit() override
    {
        RegisterOperation(REGlobal::CreateNewSceneOperation.ID, this, &SceneManagerMockModule::CreateNewSceneMock);
        RegisterOperation(REGlobal::CloseAllScenesOperation.ID, this, &SceneManagerMockModule::CloseAllScenesMock);
    }

    void CreateNewSceneMock()
    {
    }

    void CloseAllScenesMock(bool)
    {
    }
};
}

DAVA_TARC_TESTCLASS(ProjectManagerTests)
{
    const DAVA::String PROPS_KEY = "ProjectManagerProperties";
    DAVA::FilePath lastOpenedProject;
    DAVA::FilePath laspOpenedPathInSettings;
    bool reloadParticles = false;
    const DAVA::FilePath testFolder = DAVA::FilePath("~doc:/Test/");
    const DAVA::FilePath firstFakeProjectPath = DAVA::FilePath("~doc:/Test/ProjectManagerTest1/");

    DAVA::TArc::DataWrapper wrapper;
    DAVA::TArc::MockListener listener;

    ~ProjectManagerTests()
    {
        SettingsManager::SetValue(Settings::General_ReloadParticlesOnPojectOpening, DAVA::VariantType(reloadParticles));
        SettingsManager::SetValue(Settings::Internal_LastProjectPath, DAVA::VariantType(laspOpenedPathInSettings));

        DAVA::TArc::PropertiesItem propsItem = GetAccessor()->CreatePropertiesNode(PROPS_KEY);
        propsItem.Set(Settings::Internal_LastProjectPath.c_str(), DAVA::Any(lastOpenedProject));

        REConsoleModuleTestUtils::ClearTestFolder(testFolder);
    }

    DAVA_TEST (LaunchAppTest)
    {
        using namespace ::testing;
        // prepare test environment
        {
            DAVA::TArc::PropertiesItem propsItem = GetAccessor()->CreatePropertiesNode(PROPS_KEY);
            lastOpenedProject = propsItem.Get<DAVA::FilePath>(Settings::Internal_LastProjectPath.c_str());

            reloadParticles = SettingsManager::GetValue(Settings::General_ReloadParticlesOnPojectOpening).AsBool();
            SettingsManager::SetValue(Settings::General_ReloadParticlesOnPojectOpening, DAVA::VariantType(false));

            laspOpenedPathInSettings = SettingsManager::GetValue(Settings::Internal_LastProjectPath).AsFilePath();
            SettingsManager::SetValue(Settings::Internal_LastProjectPath, DAVA::VariantType(DAVA::FilePath()));

            REConsoleModuleTestUtils::CreateTestFolder(testFolder);
        }

        {
            REConsoleModuleTestUtils::CreateProjectInfrastructure(firstFakeProjectPath);
            DAVA::TArc::PropertiesItem propsItem = GetAccessor()->CreatePropertiesNode(PROPS_KEY);
            propsItem.Set(Settings::Internal_LastProjectPath.c_str(), firstFakeProjectPath);
        }

        wrapper = GetAccessor()->CreateWrapper(DAVA::ReflectedTypeDB::Get<ProjectManagerData>());
        wrapper.SetListener(&listener);

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

                             TEST_VERIFY(path == firstFakeProjectPath);
                         }));
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

        QWidget* wnd = GetWindow(REGlobal::MainWindowKey);
        QMainWindow* mainWnd = qobject_cast<QMainWindow*>(wnd);
        TEST_VERIFY(wnd != nullptr);

        QMenuBar* menu = mainWnd->menuBar();
        QMenu* fileMenu = menu->findChild<QMenu*>("File");
        QRect fileMenuRect = fileMenu->rect();

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
    DECLARE_TESTED_MODULE(ProjectManagerModule)
    DECLARE_TESTED_MODULE(LaunchModule)
    DECLARE_TESTED_MODULE(PMT::SceneManagerMockModule)
    END_TESTED_MODULES()
};