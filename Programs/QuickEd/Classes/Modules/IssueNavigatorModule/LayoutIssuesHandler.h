#pragma once

#include <Base/BaseTypes.h>
#include <UI/Layouts/UILayoutSystemListener.h>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
}

class UIControl;
class LayoutFormula;
}

class IssueNavigatorWidget;

class LayoutIssuesHandler : public DAVA::UILayoutSystemListener
{
public:
    LayoutIssuesHandler(DAVA::TArc::ContextAccessor* accessor, DAVA::int32 sectionId, IssueNavigatorWidget* widget);
    ~LayoutIssuesHandler() override;

    void OnFormulaProcessed(DAVA::UIControl* control, DAVA::Vector2::eAxis axis, const DAVA::LayoutFormula* formula) override;
    void OnFormulaRemoved(DAVA::UIControl* control, DAVA::Vector2::eAxis axis, const DAVA::LayoutFormula* formula) override;

private:
    void RemoveIssue(DAVA::UIControl* control, DAVA::Vector2::eAxis axis);
    bool IsRootControl(const DAVA::UIControl* control) const;

    DAVA::int32 sectionId = 0;
    IssueNavigatorWidget* widget = nullptr;

    DAVA::int32 nextIssueId = 0;

    DAVA::Array<DAVA::UnorderedMap<DAVA::UIControl*, DAVA::int32>, DAVA::Vector2::AXIS_COUNT> createdIssues;

    DAVA::TArc::ContextAccessor* accessor = nullptr;
};
