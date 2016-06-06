#include "REApplication.h"

#include "Main/mainwindow.h"
#include "ResourceEditorLauncher.h"
#include "Commands2/NGTCommand.h"

#include "NgtTools/Common/GlobalContext.h"
#include "QtTools/DavaGLWidget/davaglwidget.h"

#include "Deprecated/ControlsFactory.h"

#include <core_command_system/i_command_manager.hpp>
#include <core_command_system/i_history_panel.h>

REApplication::REApplication(int argc, char** argv)
    : BaseApplication(argc, argv)
    , ngtCommand(new NGTCommand())
{
}

REApplication::~REApplication() = default;

void REApplication::Run()
{
    IHistoryPanel* historyPanel = NGTLayer::queryInterface<IHistoryPanel>();
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
    StartApplication(mainWindow);

    DAVA::SafeRelease(mainWindow);
    ControlsFactory::ReleaseFonts();
}

void REApplication::GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const
{
    names.push_back(L"plg_reflection");
    names.push_back(L"plg_variant");
    names.push_back(L"plg_command_system");
    names.push_back(L"plg_serialization");
    names.push_back(L"plg_file_system");
    names.push_back(L"plg_editor_interaction");
    names.push_back(L"plg_qt_app");
    names.push_back(L"plg_qt_common");
    names.push_back(L"plg_history_ui");
}

void REApplication::OnPostLoadPugins()
{
    qApp->setOrganizationName("DAVA");
    qApp->setApplicationName("Resource Editor");

    commandManager = NGTLayer::queryInterface<ICommandManager>();
    commandManager->SetHistorySerializationEnabled(false);
    commandManager->registerCommand(ngtCommand.get());
}

void REApplication::OnPreUnloadPlugins()
{
    commandManager->deregisterCommand(ngtCommand->getId());
}
