#include "LayoutIssuesHandler.h"

#include "Modules/IssueNavigatorModule/IssueNavigatorWidget.h"

#include <UI/UIControlSystem.h>
#include <UI/Layouts/UILayoutSystem.h>
#include <UI/Layouts/LayoutFormula.h>

using namespace DAVA;

LayoutIssuesHandler::LayoutIssuesHandler(DAVA::int32 sectionId_, IssueNavigatorWidget* widget_)
    : sectionId(sectionId_)
    , widget(widget_)
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
            Issue issue;
            issue.sectionId = sectionId;
            issue.issueId = nextIssueId;
            issue.message = formula->GetErrorMessage();
            issue.packagePath = "";
            issue.pathToControl = "";
            issue.propertyName = "";

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
