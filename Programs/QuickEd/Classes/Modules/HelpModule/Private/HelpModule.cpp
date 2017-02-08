#include "Modules/HelpModule/HelpModule.h"

#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

#include <FileSystem/FileSystem.h>

#include <QDesktopServices>

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
    ContextAccessor* accessor = GetAccessor();

    QtAction* action = new QtAction(accessor, QIcon(":/Icons/help.png"), QString("QuickEd Help"));
    connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&HelpModule::UnpackHelp, this));

    ActionPlacementInfo placementInfo;
    placementInfo.AddPlacementPoint(CreateMenuPoint("Help", { InsertionParams::eInsertionMethod::AfterItem }));
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
            Logger::Error("%s", ex.what());
            DVASSERT(false && "can't unpack help docs to documents dir");
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
