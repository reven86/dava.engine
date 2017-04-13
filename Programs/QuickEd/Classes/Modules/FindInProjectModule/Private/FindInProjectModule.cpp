#include "Modules/FindInProjectModule/FindInProjectModule.h"
#include "Application/QEGlobal.h"
#include "Modules/ProjectModule/ProjectData.h"
#include "UI/Find/Filters/FindFilter.h"
#include "UI/Find/Widgets/FindInProjectDialog.h"

#include <TArc/DataProcessing/Common.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/Utils/ModuleCollection.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(FindInProjectModule)
{
    ReflectionRegistrator<FindInProjectModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void FindInProjectModule::PostInit()
{
    TArc::UI* ui = GetUI();

    TArc::ContextAccessor* accessor = GetAccessor();

    TArc::FieldDescriptor packageFieldDescr;
    packageFieldDescr.type = ReflectedTypeDB::Get<ProjectData>();
    packageFieldDescr.fieldName = FastName(ProjectData::projectPathPropertyName);

    const auto updater =
    [](const Any& fieldValue) -> Any {
        return !fieldValue.Cast<FilePath>(FilePath()).IsEmpty();
    };

    TArc::QtAction* findInProjectAction = new TArc::QtAction(accessor, QObject::tr("Find in Project..."), nullptr);
    findInProjectAction->setShortcut(Qt::SHIFT + Qt::CTRL + Qt::Key_F);
    findInProjectAction->SetStateUpdationFunction(TArc::QtAction::Enabled, packageFieldDescr, updater);

    connections.AddConnection(findInProjectAction, &QAction::triggered, MakeFunction(this, &FindInProjectModule::OnFindInProject));

    TArc::ActionPlacementInfo placementInfo(TArc::CreateMenuPoint("Find", TArc::InsertionParams(TArc::InsertionParams::eInsertionMethod::AfterItem)));
    ui->AddAction(DAVA::TArc::mainWindowKey, placementInfo, findInProjectAction);
}

void FindInProjectModule::OnFindInProject()
{
    FindInProjectDialog findInProjectDialog;
    if (findInProjectDialog.exec() == QDialog::Accepted)
    {
        InvokeOperation(QEGlobal::FindInProject.ID, std::shared_ptr<FindFilter>(findInProjectDialog.BuildFindFilter()));
    }
}

DECL_GUI_MODULE(FindInProjectModule);
