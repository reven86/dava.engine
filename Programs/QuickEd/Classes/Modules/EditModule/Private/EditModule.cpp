#include "Modules/EditModule/EditModule.h"
#include "Modules/DocumentsModule/DocumentData.h"

#include "Application/QEGlobal.h"

#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/Utils/ModuleCollection.h>

#include <Command/CommandStack.h>

const QString undoActionName("Undo");
const QString redoActionName("Redo");

void EditModule::PostInit()
{
    CreateActions();
}

void EditModule::CreateActions()
{
    using namespace DAVA;
    using namespace TArc;

    const QString toolBarName("mainToolbar");
    const QString editMenuName("Edit");
    const QString editMenuSeparatorName("undo redo separator");

    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    //Undo
    {
        QtAction* action = new QtAction(accessor, QIcon(":/Icons/edit_undo.png"), undoActionName);
        action->setShortcutContext(Qt::ApplicationShortcut);
        action->setShortcut(QKeySequence("Ctrl+Z"));

        FieldDescriptor fieldDescrCanUndo;
        fieldDescrCanUndo.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescrCanUndo.fieldName = FastName(DocumentData::canUndoPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescrCanUndo, [](const Any& fieldValue) -> Any {
            return fieldValue.CanCast<bool>() && fieldValue.Cast<bool>();
        });

        FieldDescriptor fieldDescrUndoText;
        fieldDescrUndoText.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescrUndoText.fieldName = FastName(DocumentData::undoTextPropertyName);
        action->SetStateUpdationFunction(QtAction::Text, fieldDescrUndoText, [](const Any& fieldValue) -> Any {
            QString retString = "Undo";
            if (fieldValue.CanCast<QString>())
            {
                QString text = fieldValue.Cast<QString>();
                if (text.isEmpty() == false)
                {
                    retString += ": " + text;
                }
            }
            return retString;
        });

        connections.AddConnection(action, &QAction::trigger, Bind(&EditModule::OnUndo, this));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(editMenuName, { InsertionParams::eInsertionMethod::BeforeItem }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName, { InsertionParams::eInsertionMethod::AfterItem, "documents separator" }));

        ui->AddAction(QEGlobal::windowKey, placementInfo, action);
    }

    //Redo
    {
        QtAction* action = new QtAction(accessor, QIcon(":/Icons/edit_redo.png"), redoActionName);
        action->setShortcutContext(Qt::ApplicationShortcut);
        action->setShortcuts(QList<QKeySequence>()
                             << Qt::CTRL + Qt::Key_Y
                             << Qt::CTRL + Qt::SHIFT + Qt::Key_Z);

        FieldDescriptor fieldDescrCanRedo;
        fieldDescrCanRedo.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescrCanRedo.fieldName = FastName(DocumentData::canRedoPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescrCanRedo, [](const Any& fieldValue) -> Any {
            return fieldValue.CanCast<bool>() && fieldValue.Cast<bool>();
        });

        FieldDescriptor fieldDescrUndoText;
        fieldDescrUndoText.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescrUndoText.fieldName = FastName(DocumentData::redoTextPropertyName);
        action->SetStateUpdationFunction(QtAction::Text, fieldDescrUndoText, [](const Any& fieldValue) -> Any {
            QString retString = "Redo";
            if (fieldValue.CanCast<QString>())
            {
                QString text = fieldValue.Cast<QString>();
                if (text.isEmpty() == false)
                {
                    retString += ": " + text;
                }
            }
            return retString;
        });

        connections.AddConnection(action, &QAction::trigger, Bind(&EditModule::OnRedo, this));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(editMenuName, { InsertionParams::eInsertionMethod::AfterItem, undoActionName }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName, { InsertionParams::eInsertionMethod::AfterItem, undoActionName }));

        ui->AddAction(QEGlobal::windowKey, placementInfo, action);
    }

    // Separator
    {
        QAction* separator = new QAction(editMenuSeparatorName, nullptr);
        separator->setSeparator(true);
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateToolbarPoint(editMenuName, { InsertionParams::eInsertionMethod::AfterItem, redoActionName }));
        ui->AddAction(QEGlobal::windowKey, placementInfo, separator);
    }
}

void EditModule::OnUndo()
{
    using namespace DAVA;
    using namespace TArc;

    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetActiveContext();
    DVASSERT(context != nullptr);
    DocumentData* data = context->GetData<DocumentData>();
    DVASSERT(data != nullptr);
    DVASSERT(data->commandStack->CanUndo());
    data->commandStack->Undo();
}

void EditModule::OnRedo()
{
    using namespace DAVA;
    using namespace TArc;

    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetActiveContext();
    DVASSERT(context != nullptr);
    DocumentData* data = context->GetData<DocumentData>();
    DVASSERT(data != nullptr);
    DVASSERT(data->commandStack->CanRedo());
    data->commandStack->Redo();
}

DECL_GUI_MODULE(EditModule);
