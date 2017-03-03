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

void FindController::FindAll()
{
    emit ShowFindResults(context.resultsRaw);
}

void FindController::SetFilter(std::shared_ptr<FindFilter> filter)
{
    context.filter = filter;
    context.results.clear();
    context.resultsRaw.clear();
    context.currentSelection = 0;

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

                         context.resultsRaw.push_back(item);
                     });

    for (const ControlNode* control : rootControls)
    {
        finder.Process(packagePath, control);
    }
}

void FindController::CancelFind()
{
    context.results.clear();
    context.resultsRaw.clear();
    context.currentSelection = 0;
}

void FindController::SetFindScope(const DAVA::FilePath& packagePath_, const DAVA::Vector<ControlNode*>& rootControls_)
{
    packagePath = packagePath_;
    rootControls = rootControls_;
}

void FindController::MoveSelection(int32 step)
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

        previewWidget->SelectControl(context.results[context.currentSelection]);
    }
}
