#include "UI/Find/FindInDocumentController.h"
#include "UI/Find/Finder/Finder.h"
#include "UI/Find/Widgets/FindInDocumentWidget.h"
#include "UI/mainwindow.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/DocumentsModule/DocumentsModule.h"

using namespace DAVA;

FindInDocumentController::FindInDocumentController(DocumentsModule* documentsModule_, MainWindow* mainWindow, FindInDocumentWidget* findInDocumentWidget_)
    : QObject()
    , documentsModule(documentsModule_)
    , findInDocumentWidget(findInDocumentWidget_)
{
    QObject::connect(findInDocumentWidget, &FindInDocumentWidget::OnFindFilterReady, this, &FindInDocumentController::SetFilter);
    QObject::connect(findInDocumentWidget, &FindInDocumentWidget::OnFindNext, this, &FindInDocumentController::SelectNextFindResult);
    QObject::connect(findInDocumentWidget, &FindInDocumentWidget::OnFindPrevious, this, &FindInDocumentController::SelectPreviousFindResult);
    QObject::connect(findInDocumentWidget, &FindInDocumentWidget::OnFindAll, this, &FindInDocumentController::FindAll);
    QObject::connect(findInDocumentWidget, &FindInDocumentWidget::OnStopFind, this, &FindInDocumentController::HideFindInDocumentWidget);

    QObject::connect(mainWindow, &MainWindow::FindInDocument, this, &FindInDocumentController::ShowFindInDocumentWidget);
    QObject::connect(mainWindow, &MainWindow::FindNext, this, &FindInDocumentController::SelectNextFindResult);
    QObject::connect(mainWindow, &MainWindow::FindPrevious, this, &FindInDocumentController::SelectPreviousFindResult);

    findInDocumentWidget->hide();

    TArc::ContextAccessor* accessor = documentsModule->GetAccessor();
    documentDataWrapper = accessor->CreateWrapper(ReflectedTypeDB::Get<DocumentData>());
    documentDataWrapper.SetListener(this);
}

void FindInDocumentController::ShowFindInDocumentWidget()
{
    TArc::ContextAccessor* accessor = documentsModule->GetAccessor();
    TArc::DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext != nullptr)
    {
        findInDocumentWidget->Reset();
        findInDocumentWidget->show();
        findInDocumentWidget->setFocus();
    }
}

void FindInDocumentController::HideFindInDocumentWidget()
{
    findInDocumentWidget->hide();
}

void FindInDocumentController::SelectNextFindResult()
{
    MoveSelection(+1);
}

void FindInDocumentController::SelectPreviousFindResult()
{
    MoveSelection(-1);
}

void FindInDocumentController::FindAll()
{
    documentsModule->InvokeOperation(QEGlobal::FindInDocument.ID, context.filter);
}

void FindInDocumentController::SetFilter(std::shared_ptr<FindFilter> filter)
{
    context.filter = filter;
    context.results.clear();
    context.currentSelection = -1;

    Finder finder(filter, nullptr);

    QObject::connect(&finder, &Finder::ItemFound,
                     [this](const FindItem& item)
                     {
                         for (const String& path : item.GetControlPaths())
                         {
                             context.results.push_back(path);
                         }
                     });

    TArc::ContextAccessor* accessor = documentsModule->GetAccessor();
    TArc::DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext != nullptr)
    {
        DocumentData* data = activeContext->GetData<DocumentData>();

        const SortedControlNodeSet& editedRootControls = data->GetEditedRootControls();

        finder.Process(data->GetPackageNode(), editedRootControls);
    }
}

void FindInDocumentController::MoveSelection(int32 step)
{
    if (!context.results.empty())
    {
        context.currentSelection += step;

        if (context.currentSelection < 0)
        {
            context.currentSelection = static_cast<int32>(context.results.size() - 1);
        }
        else if (context.currentSelection >= context.results.size())
        {
            context.currentSelection = 0;
        }

        TArc::ContextAccessor* accessor = documentsModule->GetAccessor();
        TArc::DataContext* activeContext = accessor->GetActiveContext();
        DVASSERT(activeContext != nullptr);
        DocumentData* data = activeContext->GetData<DocumentData>();

        const QString& path = data->GetPackageAbsolutePath();
        const QString& name = QString::fromStdString(context.results[context.currentSelection]);
        documentsModule->InvokeOperation(QEGlobal::SelectControl.ID, path, name);
    }
}

void FindInDocumentController::OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    const bool editedRootControlsChanged = std::find(fields.begin(), fields.end(), String(DocumentData::editedRootControlsPropertyName)) != fields.end();

    if (editedRootControlsChanged)
    {
        HideFindInDocumentWidget();

        context.results.clear();
        context.currentSelection = -1;
    }
}
