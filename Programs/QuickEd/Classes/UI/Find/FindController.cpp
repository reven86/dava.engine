#include "UI/Find/FindController.h"
#include "Document/Document.h"
#include "UI/Find/Finder.h"
#include "UI/UIEvent.h"
#include "UI/UIControl.h"
#include "UI/Preview/PreviewWidget.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/VisibleValueProperty.h"
#include "Logger/Logger.h"

using namespace DAVA;

FindController::FindController(PreviewWidget* previewWidget_)
    : QObject(previewWidget_)
    , previewWidget(previewWidget_)
{
}

FindController::~FindController()
{
}

void FindController::SelectNextFindResult()
{
    MoveSelection(+1);
}

void FindController::SelectPrevFindResult()
{
    MoveSelection(-1);
}

void FindController::FindInDocument(std::shared_ptr<FindFilter> filter)
{
    if (document)
    {
        context.filter = filter;
        context.results.clear();

        Finder finder(filter, nullptr);

        QObject::connect(&finder, &Finder::ItemFound,
                         [this](const FindItem& item)
                         {
                             for (const String& path : item.GetControlPaths())
                             {
                                 context.results.push_back(path);
                                 Logger::Debug("%s %s",
                                               __FUNCTION__,
                                               path.c_str());
                             }
                         });

        QObject::connect(&finder, &Finder::Finished,
                         [this]()
                         {
                             UpdateHighlight();
                         });

        finder.Process(document->GetPackage());
    }
}

void FindController::OnDocumentChanged(Document* document_)
{
    document = document_;
}

void FindController::CancelFind()
{
    context.results.clear();
    context.currentSelection = -1;

    UpdateHighlight();
}

void FindController::UpdateHighlight()
{
    SelectedControls controls;

    for (const String& path : context.results)
    {
        if (ControlNode* node = previewWidget->systemsManager->GetControlNodeByPath(path))
        {
            controls.insert(node);
        }
    }

    previewWidget->systemsManager->searchResultsChanged.Emit(controls);
}

void FindController::MoveSelection(int32 step)
{
    if (!context.results.empty())
    {
        context.currentSelection += step;
        context.currentSelection = context.currentSelection % context.results.size();

        if (ControlNode* node = previewWidget->systemsManager->GetControlNodeByPath(context.results[context.currentSelection]))
        {
            previewWidget->systemsManager->SelectNode(node);
        }
    }
}
