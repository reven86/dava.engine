#include "LayoutIssuesHandler.h"

#include "Modules/IssueNavigatorModule/IssueNavigatorWidget.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

#include <UI/UIControlSystem.h>
#include <UI/UIControl.h>
#include <UI/Layouts/UILayoutSystem.h>
#include <UI/Layouts/LayoutFormula.h>

#include <TArc/Core/ContextAccessor.h>

using namespace DAVA;

LayoutIssuesHandler::LayoutIssuesHandler(DAVA::TArc::ContextAccessor* accessor_, DAVA::int32 sectionId_, IssueNavigatorWidget* widget_)
    : sectionId(sectionId_)
    , widget(widget_)
    , accessor(accessor_)
{
    UIControlSystem::Instance()->GetLayoutSystem()->AddListener(this);
}

LayoutIssuesHandler::~LayoutIssuesHandler()
{
    UIControlSystem::Instance()->GetLayoutSystem()->RemoveListener(this);
}

void LayoutIssuesHandler::OnFormulaProcessed(UIControl* control, Vector2::eAxis axis, const LayoutFormula* formula)
{
    if (formula->HasError())
    {
        auto it = createdIssues[axis].find(control);
        if (it != createdIssues[axis].end())
        {
            widget->ChangeMessage(sectionId, it->second, formula->GetErrorMessage());
        }
        else
        {
            const DocumentData* data = accessor->GetActiveContext()->GetData<DocumentData>();
            DVASSERT(data != nullptr);

            String pathToControl = control->GetName().c_str();

            UIControl* p = control->GetParent();
            while (p != nullptr)
            {
                String n = "";
                if (p->GetName().IsValid())
                {
                    n = p->GetName().c_str();
                }
                pathToControl = n + "/" + pathToControl;

                if (IsRootControl(p))
                {
                    break;
                }
                p = p->GetParent();
            }

            Issue issue;
            issue.sectionId = sectionId;
            issue.issueId = nextIssueId;
            issue.message = formula->GetErrorMessage();
            issue.packagePath = data->GetPackagePath().GetFrameworkPath();
            issue.pathToControl = pathToControl;
            issue.propertyName = axis == Vector2::AXIS_X ? "SizePolicy/horizontalFormula" : "SizePolicy/verticalFormula";

            nextIssueId++;
            widget->AddIssue(issue);

            createdIssues[axis][control] = issue.issueId;
        }
    }
    else
    {
        RemoveIssue(control, axis);
    }
}

void LayoutIssuesHandler::OnFormulaRemoved(UIControl* control, Vector2::eAxis axis, const LayoutFormula* formula)
{
    RemoveIssue(control, axis);
}

void LayoutIssuesHandler::RemoveIssue(DAVA::UIControl* control, DAVA::Vector2::eAxis axis)
{
    auto it = createdIssues[axis].find(control);
    if (it != createdIssues[axis].end())
    {
        widget->RemoveIssue(sectionId, it->second);
        createdIssues[axis].erase(it);
    }
}

bool LayoutIssuesHandler::IsRootControl(const UIControl* control) const
{
    const DocumentData* data = accessor->GetActiveContext()->GetData<DocumentData>();
    DVASSERT(data != nullptr);

    PackageControlsNode* controls = data->GetPackageNode()->GetPackageControlsNode();
    for (ControlNode* controlNode : *controls)
    {
        if (controlNode->GetControl() == control)
        {
            return true;
        }
    }

    PackageControlsNode* prototypes = data->GetPackageNode()->GetPrototypes();
    for (ControlNode* controlNode : *prototypes)
    {
        if (controlNode->GetControl() == control)
        {
            return true;
        }
    }
    return false;
}
