#include "QEApplication.h"
#include "EditorCore.h"


#include "Document/CommandsBase/WGTCommand.h"
#include "NgtTools/Common/GlobalContext.h"

#include <core_command_system/i_command_manager.hpp>
#include <core_command_system/i_history_panel.h>
#include <QtTools/DavaGLWidget/davaglwidget.h>

QEApplication::QEApplication(int argc, char** argv)
    : BaseApplication(argc, argv)
    , ngtCommand(new WGTCommand())
{
}

QEApplication::~QEApplication() = default;

int QEApplication::Run()
{
    IHistoryPanel* historyPanel = NGTLayer::queryInterface<IHistoryPanel>();
    if (historyPanel != nullptr)
    {
        historyPanel->setClearButtonVisible(false);
        historyPanel->setMakeMacroButtonVisible(false);
    }
    EditorCore editorCore;

    editorCore.Start();
    return StartApplication(editorCore.GetMainWindow());
}

void QEApplication::GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const
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

void QEApplication::OnPostLoadPugins()
{
    qApp->setOrganizationName("DAVA");
    qApp->setApplicationName("QuickEd");

    commandManager = NGTLayer::queryInterface<ICommandManager>();
    commandManager->SetHistorySerializationEnabled(false);
    commandManager->registerCommand(ngtCommand.get());
}

void QEApplication::OnPreUnloadPlugins()
{
    commandManager->deregisterCommand(ngtCommand->getId());
}
