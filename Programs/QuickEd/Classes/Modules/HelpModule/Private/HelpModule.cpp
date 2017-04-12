#include "Modules/HelpModule/HelpModule.h"
#include "Application/QEGlobal.h"

#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

#include <FileSystem/FileSystem.h>

#include <QDesktopServices>

DAVA_VIRTUAL_REFLECTION_IMPL(HelpModule)
{
    DAVA::ReflectionRegistrator<HelpModule>::Begin()
    .ConstructorByPointer()
    .End();
}

namespace HelpModuleDetails
{
static const DAVA::String helpDirectory("~doc:/Help/");
}

void HelpModule::PostInit()
{
    UnpackHelp();
    CreateActions();
}

void HelpModule::CreateActions()
{
    using namespace DAVA::TArc;

    QAction* action = new QAction(QIcon(":/Icons/help.png"), "QuickEd Help", nullptr);
    action->setShortcut(Qt::Key_F1);
    connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&HelpModule::OnShowHelp, this));

    ActionPlacementInfo placementInfo;
    placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuHelp, { InsertionParams::eInsertionMethod::BeforeItem }));
    GetUI()->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
}

void HelpModule::UnpackHelp()
{
    using namespace DAVA;
    FileSystem* fs = GetAccessor()->GetEngineContext()->fileSystem;
    FilePath docsPath(HelpModuleDetails::helpDirectory);
    if (!fs->Exists(docsPath))
    {
        try
        {
            ResourceArchive helpRA("~res:/QuickEd/Help.docs");

            fs->DeleteDirectory(docsPath);
            fs->CreateDirectory(docsPath, true);

            helpRA.UnpackToFolder(docsPath);
        }
        catch (std::exception& ex)
        {
            Logger::Error("can not unpack help. Reason is: %s", ex.what());
        }
    }
}

void HelpModule::OnShowHelp()
{
    using namespace DAVA;
    FilePath docsPath = HelpModuleDetails::helpDirectory + "index.html";
    QString docsFile = QString::fromStdString("file:///" + docsPath.GetAbsolutePathname());
    QDesktopServices::openUrl(QUrl(docsFile));
}

DECL_GUI_MODULE(HelpModule);
