#include "UILayoutSystem.h"

#include "Concurrency/Thread.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Entity/Component.h"
#include "Render/Renderer.h"
#include "UI/Layouts/AnchorLayoutAlgorithm.h"
#include "UI/Layouts/FlowLayoutAlgorithm.h"
#include "UI/Layouts/LinearLayoutAlgorithm.h"
#include "UI/Layouts/SizeMeasuringAlgorithm.h"
#include "UI/Layouts/UIAnchorComponent.h"
#include "UI/Layouts/UIFlowLayoutComponent.h"
#include "UI/Layouts/UILinearLayoutComponent.h"
#include "UI/Layouts/UISizePolicyComponent.h"
#include "UI/Layouts/UILayoutSourceRectComponent.h"
#include "UI/Layouts/UILayoutIsolationComponent.h"
#include "UI/Layouts/LayoutFormula.h"
#include "UI/Layouts/UILayoutSystemListener.h"
#include "UI/UIControl.h"
#include "UI/UIScreen.h"
#include "UI/UIScreenTransition.h"

namespace DAVA
{
UILayoutSystem::UILayoutSystem()
{
}

UILayoutSystem::~UILayoutSystem()
{
    DVASSERT(listeners.empty());
}

void UILayoutSystem::Process(float32 elapsedTime)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::UI_LAYOUT_SYSTEM);

    if (!IsAutoupdatesEnabled())
        return;

    CheckDirty();

    if (!needUpdate)
        return;

    if (currentScreenTransition.Valid())
    {
        ProcessControlHierarhy(currentScreenTransition.Get());
    }
    else if (currentScreen.Valid())
    {
        ProcessControlHierarhy(currentScreen.Get());
    }

    if (popupContainer.Valid())
    {
        ProcessControlHierarhy(popupContainer.Get());
    }
}

void UILayoutSystem::UnregisterControl(UIControl* control)
{
    UISizePolicyComponent* sizePolicyComponent = control->GetComponent<UISizePolicyComponent>();
    if (sizePolicyComponent != nullptr)
    {
        for (int32 axis = Vector2::AXIS_X; axis < Vector2::AXIS_COUNT; axis++)
        {
            LayoutFormula* formula = sizePolicyComponent->GetFormula(axis);
            if (formula != nullptr)
            {
                formula->MarkChanges();
                for (UILayoutSystemListener* listener : listeners)
                {
                    listener->OnFormulaRemoved(control, static_cast<Vector2::eAxis>(axis), formula);
                }
            }
        }
    }
}

void UILayoutSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    UISizePolicyComponent* sizePolicyComponent = CastIfEqual<UISizePolicyComponent*>(component);
    if (sizePolicyComponent != nullptr)
    {
        for (int32 axis = Vector2::AXIS_X; axis < Vector2::AXIS_COUNT; axis++)
        {
            LayoutFormula* formula = sizePolicyComponent->GetFormula(axis);
            if (formula != nullptr)
            {
                for (UILayoutSystemListener* listener : listeners)
                {
                    listener->OnFormulaRemoved(control, static_cast<Vector2::eAxis>(axis), formula);
                }
            }
        }
    }
}

void UILayoutSystem::ForceProcessControl(float32 elapsedTime, UIControl* control)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::UI_LAYOUT_SYSTEM);

    if (!needUpdate && !dirty)
        return;

    ProcessControlHierarhy(control);
}

void UILayoutSystem::SetCurrentScreen(const RefPtr<UIScreen>& screen)
{
    currentScreen = screen;
}

void UILayoutSystem::SetCurrentScreenTransition(const RefPtr<UIScreenTransition>& screenTransition)
{
    currentScreenTransition = screenTransition;
}

void UILayoutSystem::SetPopupContainer(const RefPtr<UIControl>& _popupContainer)
{
    popupContainer = _popupContainer;
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
    bool layoutDirty = control->IsLayoutDirty();
    bool orderDirty = control->IsLayoutOrderDirty();
    bool positionDirty = control->IsLayoutPositionDirty();
    control->ResetLayoutDirty();

    if (layoutDirty || (orderDirty && HaveToLayoutAfterReorder(control)) || (positionDirty && control->GetParent() && control->GetParent()->GetComponent(Type::Instance<UILayoutSourceRectComponent>())))
    {
        UIControl* container = FindNotDependentOnChildrenControl(control);
        ApplyLayout(container);

        for (UILayoutSystemListener* listener : listeners)
        {
            listener->OnControlLayouted(container);
        }
    }
    else if (positionDirty && HaveToLayoutAfterReposition(control))
    {
        UIControl* container = control->GetParent();
        ApplyLayoutNonRecursive(container);

        for (UILayoutSystemListener* listener : listeners)
        {
            listener->OnControlLayouted(container);
        }
    }
}

void UILayoutSystem::ManualApplyLayout(UIControl* control)
{
    ApplyLayout(control);
}

bool UILayoutSystem::IsAutoupdatesEnabled() const
{
    return autoupdatesEnabled;
}

void UILayoutSystem::SetAutoupdatesEnabled(bool enabled)
{
    autoupdatesEnabled = enabled;
}

void UILayoutSystem::ApplyLayout(UIControl* control)
{
    DVASSERT(Thread::IsMainThread() || autoupdatesEnabled == false);

    CollectControls(control, true);

    ProcessAxis(Vector2::AXIS_X, true);
    ProcessAxis(Vector2::AXIS_Y, true);

    ApplySizesAndPositions();

    layoutData.clear();
}

void UILayoutSystem::ApplyLayoutNonRecursive(UIControl* control)
{
    DVASSERT(Thread::IsMainThread() || autoupdatesEnabled == false);

    CollectControls(control, false);

    ProcessAxis(Vector2::AXIS_X, false);
    ProcessAxis(Vector2::AXIS_Y, false);

    ApplyPositions();

    layoutData.clear();
}

void UILayoutSystem::AddListener(UILayoutSystemListener* listener)
{
    auto it = std::find(listeners.begin(), listeners.end(), listener);
    if (it == listeners.end())
    {
        listeners.push_back(listener);
    }
    else
    {
        DVASSERT(false);
    }
}

void UILayoutSystem::RemoveListener(UILayoutSystemListener* listener)
{
    auto it = std::find(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end())
    {
        listeners.erase(it);
    }
    else
    {
        DVASSERT(false);
    }
}

UIControl* UILayoutSystem::FindNotDependentOnChildrenControl(UIControl* control) const
{
    UIControl* result = control;
    while (result->GetParent() != nullptr && result->GetComponentCount(Type::Instance<UILayoutIsolationComponent>()) == 0)
    {
        UISizePolicyComponent* sizePolicy = result->GetParent()->GetComponent<UISizePolicyComponent>();
        if ((sizePolicy != nullptr && (sizePolicy->IsDependsOnChildren(Vector2::AXIS_X) || sizePolicy->IsDependsOnChildren(Vector2::AXIS_Y))) ||
            result->GetParent()->GetComponent(Type::Instance<UILayoutSourceRectComponent>()) != nullptr)
        {
            result = result->GetParent();
        }
        else
        {
            break;
        }
    }

    if (result->GetParent() != nullptr && result->GetComponentCount(Type::Instance<UILayoutIsolationComponent>()) == 0)
    {
        result = result->GetParent();
    }

    return result;
}

bool UILayoutSystem::HaveToLayoutAfterReorder(const UIControl* control) const
{
    if (control->GetComponentCount<UILinearLayoutComponent>() || control->GetComponentCount<UIFlowLayoutComponent>())
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

bool UILayoutSystem::HaveToLayoutAfterReposition(const UIControl* control) const
{
    const UIControl* parent = control->GetParent();
    if (parent == nullptr)
    {
        return false;
    }

    if (control->GetComponentCount<UIAnchorComponent>())
    {
        return true;
    }

    if (parent->GetComponentCount<UILinearLayoutComponent>() || parent->GetComponentCount<UIFlowLayoutComponent>())
    {
        return true;
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

    int32 childrenCount = 0;

    for (UIControl* child : children)
    {
        if (child->GetComponentCount(Type::Instance<UILayoutIsolationComponent>()) == 0)
        {
            layoutData.emplace_back(ControlLayoutData(child));
            childrenCount++;
        }
    }

    layoutData[parentIndex].SetFirstChildIndex(index);
    layoutData[parentIndex].SetLastChildIndex(index + childrenCount - 1);

    if (recursive)
    {
        for (UIControl* child : children)
        {
            if (child->GetComponentCount(Type::Instance<UILayoutIsolationComponent>()) == 0)
            {
                CollectControlChildren(child, index, recursive);
                index++;
            }
        }
    }
}

void UILayoutSystem::ProcessAxis(Vector2::eAxis axis, bool processSizes)
{
    if (processSizes)
    {
        DoMeasurePhase(axis);
    }
    DoLayoutPhase(axis);
}

void UILayoutSystem::DoMeasurePhase(Vector2::eAxis axis)
{
    int32 lastIndex = static_cast<int32>(layoutData.size() - 1);
    for (int32 index = lastIndex; index >= 0; index--)
    {
        UISizePolicyComponent* sizePolicy = layoutData[index].GetControl()->GetComponent<UISizePolicyComponent>();
        if (sizePolicy != nullptr)
        {
            SizeMeasuringAlgorithm(layoutData, layoutData[index], axis, sizePolicy).Apply();
        }
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

        UISizePolicyComponent* sizePolicy = (*it).GetControl()->GetComponent<UISizePolicyComponent>();
        if (sizePolicy != nullptr)
        {
            LayoutFormula* formula = sizePolicy->GetFormula(axis);
            if (formula != nullptr && formula->HasChanges())
            {
                formula->ResetChanges();
                if (formula->IsEmpty())
                {
                    for (UILayoutSystemListener* listener : listeners)
                    {
                        listener->OnFormulaRemoved(sizePolicy->GetControl(), axis, formula);
                    }
                    sizePolicy->RemoveFormula(axis);
                }
                else
                {
                    for (UILayoutSystemListener* listener : listeners)
                    {
                        listener->OnFormulaProcessed(sizePolicy->GetControl(), axis, formula);
                    }
                }
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

void UILayoutSystem::ProcessControlHierarhy(UIControl* control)
{
    ProcessControl(control);

    // TODO: For now game has many places where changes in layouts can
    // change hierarchy of controls. In future client want fix this places,
    // after that this code should be replaced by simple for-each.
    const List<UIControl*>& children = control->GetChildren();
    auto it = children.begin();
    auto endIt = children.end();
    while (it != endIt)
    {
        control->isIteratorCorrupted = false;
        ProcessControlHierarhy(*it);
        if (control->isIteratorCorrupted)
        {
            it = children.begin();
            continue;
        }
        ++it;
    }
}
}
