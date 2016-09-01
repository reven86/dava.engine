#include "UILayoutSystem.h"

#include "UILinearLayoutComponent.h"
#include "UIFlowLayoutComponent.h"
#include "UIAnchorComponent.h"
#include "UISizePolicyComponent.h"

#include "SizeMeasuringAlgorithm.h"
#include "LinearLayoutAlgorithm.h"
#include "FlowLayoutAlgorithm.h"
#include "AnchorLayoutAlgorithm.h"

#include "UI/UIControl.h"
#include "UI/Styles/UIStyleSheetPropertyDataBase.h"

#include "Concurrency/Thread.h"
#include "Debug/Profiler.h"

namespace DAVA
{
UILayoutSystem::UILayoutSystem()
{
}

UILayoutSystem::~UILayoutSystem()
{
}

bool UILayoutSystem::IsRtl() const
{
    return isRtl;
}

void UILayoutSystem::SetRtl(bool rtl)
{
    isRtl = rtl;
}

void UILayoutSystem::ProcessControl(UIControl* control)
{
    if (!IsAutoupdatesEnabled())
        return;

    //TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "UILayoutSystem::ProcessControl");

    bool dirty = control->IsLayoutDirty();
    bool orderDirty = control->IsLayoutOrderDirty();
    bool positionDirty = control->IsLayoutPositionDirty();
    control->ResetLayoutDirty();

    if (dirty || orderDirty && LayoutAfterReorder(control))
    {
        ApplyLayout(control, true);
    }
    else if (positionDirty && LayoutAfterReposition(control))
    {
        ApplyLayoutNonRecursive(control);
    }

    //TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "UILayoutSystem::ProcessControl");
}

void UILayoutSystem::ManualApplyLayout(UIControl* control)
{
    ApplyLayout(control, false);
}

bool UILayoutSystem::IsAutoupdatesEnabled() const
{
    return autoupdatesEnabled;
}

void UILayoutSystem::SetAutoupdatesEnabled(bool enabled)
{
    autoupdatesEnabled = enabled;
}

void UILayoutSystem::ApplyLayout(UIControl* control, bool considerDenendenceOnChildren)
{
    TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "UILayoutSystem::ApplyLayout");
    //const char* ctrlName = control->GetName().c_str() ? control->GetName().c_str() : "";
    //TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", ctrlName);
    DVASSERT(Thread::IsMainThread() || autoupdatesEnabled == false);

    UIControl* container = control;
    if (considerDenendenceOnChildren)
    {
        container = FindNotDependentOnChildrenControl(container);
    }

    CollectControls(container, true);

    ProcessAxis(Vector2::AXIS_X);
    ProcessAxis(Vector2::AXIS_Y);

    ApplySizesAndPositions();

    layoutData.clear();
    //TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", ctrlName);
    TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "UILayoutSystem::ApplyLayout");
}

void UILayoutSystem::ApplyLayoutNonRecursive(UIControl* control)
{
    TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "UILayoutSystem::ApplyLayoutNonRecursive");
    DVASSERT(Thread::IsMainThread() || autoupdatesEnabled == false);

    UIControl* container = control->GetParent();

    CollectControls(container, false);

    ProcessAxis(Vector2::AXIS_X);
    ProcessAxis(Vector2::AXIS_Y);

    ApplyPositions();

    layoutData.clear();
    TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "UILayoutSystem::ApplyLayoutNonRecursive");
}

UIControl* UILayoutSystem::FindNotDependentOnChildrenControl(UIControl* control) const
{
    UIControl* result = control;
    while (result->GetParent() != nullptr)
    {
        UISizePolicyComponent* sizePolicy = result->GetParent()->GetComponent<UISizePolicyComponent>();
        if (sizePolicy != nullptr && (sizePolicy->IsDependsOnChildren(Vector2::AXIS_X) || sizePolicy->IsDependsOnChildren(Vector2::AXIS_Y)))
        {
            result = result->GetParent();
        }
        else
        {
            break;
        }
    }

    if (result->GetParent())
    {
        result = result->GetParent();
    }

    return result;
}

bool UILayoutSystem::LayoutAfterReorder(const UIControl* control) const
{
    static const uint64 sensitiveComponents = UIComponent::LINEAR_LAYOUT_COMPONENT | UIComponent::FLOW_LAYOUT_COMPONENT;
    if ((control->GetAvailableComponentFlags() & sensitiveComponents) != 0)
    {
        return true;
    }

    UISizePolicyComponent* policy = control->GetComponent<UISizePolicyComponent>();
    if (policy)
    {
        return policy->IsDependsOnChildren(Vector2::AXIS_X) || policy->IsDependsOnChildren(Vector2::AXIS_Y);
    }

    return false;
}

bool UILayoutSystem::LayoutAfterReposition(const UIControl* control) const
{
    const UIControl* parent = control->GetParent();
    if (parent == nullptr)
    {
        return false;
    }

    if ((control->GetAvailableComponentFlags() & UIComponent::ANCHOR_COMPONENT) != 0)
    {
        return true;
    }

    static const uint64 parentComponents = UIComponent::LINEAR_LAYOUT_COMPONENT | UIComponent::FLOW_LAYOUT_COMPONENT;
    if ((parent->GetAvailableComponentFlags() & parentComponents) != 0)
    {
        return true;
    }

    if ((control->GetAvailableComponentFlags() & UIComponent::SIZE_POLICY_COMPONENT) != 0)
    {
        UISizePolicyComponent* policy = control->GetComponent<UISizePolicyComponent>();
        bool res = policy->GetHorizontalPolicy() == UISizePolicyComponent::PERCENT_OF_PARENT ||
        policy->GetVerticalPolicy() == UISizePolicyComponent::PERCENT_OF_PARENT;
        return res;
    }

    return false;
}

void UILayoutSystem::CollectControls(UIControl* control, bool recursive)
{
    layoutData.clear();
    layoutData.emplace_back(ControlLayoutData(control));
    CollectControlChildren(control, 0, recursive);
}

void UILayoutSystem::CollectControlChildren(UIControl* control, int32 parentIndex, bool recursive)
{
    int32 index = static_cast<int32>(layoutData.size());
    const List<UIControl*>& children = control->GetChildren();

    layoutData[parentIndex].SetFirstChildIndex(index);
    layoutData[parentIndex].SetLastChildIndex(index + static_cast<int32>(children.size() - 1));

    for (UIControl* child : children)
    {
        layoutData.emplace_back(ControlLayoutData(child));
    }

    if (recursive)
    {
        for (UIControl* child : children)
        {
            CollectControlChildren(child, index, recursive);
            index++;
        }
    }
}

void UILayoutSystem::ProcessAxis(Vector2::eAxis axis)
{
    DoMeasurePhase(axis);
    DoLayoutPhase(axis);
}

void UILayoutSystem::DoMeasurePhase(Vector2::eAxis axis)
{
    int32 lastIndex = static_cast<int32>(layoutData.size() - 1);
    for (int32 index = lastIndex; index >= 0; index--)
    {
        SizeMeasuringAlgorithm(layoutData).Apply(layoutData[index], axis);
    }
}

void UILayoutSystem::DoLayoutPhase(Vector2::eAxis axis)
{
    for (auto it = layoutData.begin(); it != layoutData.end(); ++it)
    {
        UIFlowLayoutComponent* flowLayoutComponent = it->GetControl()->GetComponent<UIFlowLayoutComponent>();
        if (flowLayoutComponent && flowLayoutComponent->IsEnabled())
        {
            FlowLayoutAlgorithm(layoutData, isRtl).Apply(*it, axis);
        }
        else
        {
            UILinearLayoutComponent* linearLayoutComponent = it->GetControl()->GetComponent<UILinearLayoutComponent>();
            if (linearLayoutComponent != nullptr && linearLayoutComponent->IsEnabled() && linearLayoutComponent->GetAxis() == axis)
            {
                LinearLayoutAlgorithm alg(layoutData, isRtl);

                bool inverse = linearLayoutComponent->IsInverse();
                if (isRtl && linearLayoutComponent->IsUseRtl() && linearLayoutComponent->GetAxis() == Vector2::AXIS_X)
                {
                    inverse = !inverse;
                }
                alg.SetInverse(inverse);
                alg.SetSkipInvisible(linearLayoutComponent->IsSkipInvisibleControls());

                alg.SetPadding(linearLayoutComponent->GetPadding());
                alg.SetSpacing(linearLayoutComponent->GetSpacing());

                alg.SetDynamicPadding(linearLayoutComponent->IsDynamicPadding());
                alg.SetDynamicSpacing(linearLayoutComponent->IsDynamicSpacing());

                alg.Apply(*it, axis);
            }
            else
            {
                AnchorLayoutAlgorithm(layoutData, isRtl).Apply(*it, axis, false);
            }
        }
    }
}

void UILayoutSystem::ApplySizesAndPositions()
{
    for (ControlLayoutData& data : layoutData)
    {
        data.ApplyLayoutToControl();
    }
}

void UILayoutSystem::ApplyPositions()
{
    for (ControlLayoutData& data : layoutData)
    {
        data.ApplyOnlyPositionLayoutToControl();
    }
}
}
