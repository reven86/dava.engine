#include "REApplication.h"

#include "Main/mainwindow.h"
#include "ResourceEditorLauncher.h"
#include "NgtTools/Commands/WGTCommand.h"

#include "NgtTools/Commands/WGTCommand.h"
#include "NgtTools/Common/GlobalContext.h"
#include "NgtTools/Application/NGTCmdLineParser.h"
#include "QtTools/DavaGLWidget/davaglwidget.h"
#include "QtTools/Utils/Themes/Themes.h"

#include "Preferences/PreferencesStorage.h"
#include "Deprecated/ControlsFactory.h"

#include <core_command_system/i_command_manager.hpp>
#include <core_command_system/i_history_panel.h>

#include <core_reflection/i_definition_manager.hpp>
#include <core_ui_framework/i_ui_framework.hpp>

REApplication::REApplication(int argc, char** argv)
    : BaseApplication(argc, argv)
    , wgtCommand(new WGTCommand())
{
}

REApplication::~REApplication() = default;

int REApplication::Run()
{
    wgt::IHistoryPanel* historyPanel = NGTLayer::queryInterface<wgt::IHistoryPanel>();
    if (historyPanel != nullptr)
    {
        historyPanel->setClearButtonVisible(false);
        historyPanel->setMakeMacroButtonVisible(false);
    }

    // create and init UI
    ResourceEditorLauncher launcher;
    mainWindow = new QtMainWindow(GetComponentContext());

    mainWindow->EnableGlobalTimeout(true);
    DavaGLWidget* glWidget = mainWindow->GetSceneWidget()->GetDavaWidget();

    QObject::connect(glWidget, &DavaGLWidget::Initialized, &launcher, &ResourceEditorLauncher::Launch);
    int exitCode = StartApplication(mainWindow);

    DAVA::SafeRelease(mainWindow);
    ControlsFactory::ReleaseFonts();
    return exitCode;
}

void REApplication::GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const
{
    names.push_back(L"plg_variant");
    names.push_back(L"plg_reflection");
    names.push_back(L"plg_command_system");
    names.push_back(L"plg_serialization");
    names.push_back(L"plg_file_system");
    names.push_back(L"plg_editor_interaction");
    names.push_back(L"plg_qt_app");
    names.push_back(L"plg_qt_common");
    names.push_back(L"plg_history_ui");
}

void REApplication::OnPostLoadPlugins()
{
    qApp->setOrganizationName("DAVA");
    qApp->setApplicationName("Resource Editor");

    commandManager = NGTLayer::queryInterface<wgt::ICommandManager>();
    commandManager->SetHistorySerializationEnabled(false);
    commandManager->registerCommand(wgtCommand.get());

    wgt::IUIFramework* uiFramework = NGTLayer::queryInterface<wgt::IUIFramework>();
    DVASSERT(uiFramework != nullptr);

    wgt::IDefinitionManager* defManager = NGTLayer::queryInterface<wgt::IDefinitionManager>();
    DVASSERT(defManager);

    componentProvider.reset(new NGTLayer::ComponentProvider(*defManager));
    uiFramework->registerComponentProvider(*componentProvider);

    const char* settingsPath = "ResourceEditorSettings.archive";
    DAVA::FilePath localPrefrencesPath(DAVA::FileSystem::Instance()->GetCurrentDocumentsDirectory() + settingsPath);
    PreferencesStorage::Instance()->SetupStoragePath(localPrefrencesPath);

    Themes::InitFromQApplication();

    BaseApplication::OnPostLoadPlugins();
}

void REApplication::OnPreUnloadPlugins()
{
    commandManager->deregisterCommand(wgtCommand->getId());
}

bool REApplication::OnRequestCloseApp()
{
    return mainWindow->CanBeClosed();
}

void REApplication::ConfigureLineCommand(NGTLayer::NGTCmdLineParser& lineParser)
{
    lineParser.addParam("preferenceFolder", DAVA::FileSystem::Instance()->GetCurrentDocumentsDirectory().GetAbsolutePathname());
}
