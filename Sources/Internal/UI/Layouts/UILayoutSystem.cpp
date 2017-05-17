#include "UILayoutSystem.h"

#include "Concurrency/Thread.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Entity/Component.h"
#include "UI/Layouts/Private/Layouter.h"
#include "UI/Layouts/UISizePolicyComponent.h"
#include "UI/Layouts/LayoutFormula.h"
#include "UI/Layouts/UILayoutSystemListener.h"
#include "UI/UIControl.h"
#include "UI/UIScreen.h"
#include "UI/UIScreenTransition.h"

namespace DAVA
{
UILayoutSystem::UILayoutSystem()
    : sharedLayouter(std::make_unique<Layouter>())
{
    sharedLayouter->SetRtl(isRtl);
    sharedLayouter->onFormulaProcessed = [this](UIControl* control, Vector2::eAxis axis, const LayoutFormula* formula)
    {
        for (UILayoutSystemListener* listener : listeners)
        {
            listener->OnFormulaProcessed(control, axis, formula);
        }
    };

    sharedLayouter->onFormulaRemoved = [this](UIControl* control, Vector2::eAxis axis, const LayoutFormula* formula)
    {
        for (UILayoutSystemListener* listener : listeners)
        {
            listener->OnFormulaRemoved(control, axis, formula);
        }
    };
}

UILayoutSystem::~UILayoutSystem()
{
    DVASSERT(listeners.empty());
}

void UILayoutSystem::Process(float32 elapsedTime)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::UI_LAYOUT_SYSTEM);

    DVASSERT(Thread::IsMainThread());

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
    if (component->GetType() == UIComponent::SIZE_POLICY_COMPONENT)
    {
        UISizePolicyComponent* sizePolicyComponent = DynamicTypeCheck<UISizePolicyComponent*>(component);
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
    sharedLayouter->SetRtl(isRtl);
}

void UILayoutSystem::ProcessControl(UIControl* control)
{
    if (!IsAutoupdatesEnabled())
        return;

    bool layoutDirty = control->IsLayoutDirty();
    bool orderDirty = control->IsLayoutOrderDirty();
    bool positionDirty = control->IsLayoutPositionDirty();
    control->ResetLayoutDirty();

    if (layoutDirty || (orderDirty && HaveToLayoutAfterReorder(control)) || (positionDirty && control->GetParent() && control->GetParent()->GetComponent(UIComponent::LAYOUT_SOURCE_RECT_COMPONENT)))
    {
        UIControl* container = FindNotDependentOnChildrenControl(control);
        sharedLayouter->ApplyLayout(container);

        for (UILayoutSystemListener* listener : listeners)
        {
            listener->OnControlLayouted(container);
        }
    }
    else if (positionDirty && HaveToLayoutAfterReposition(control))
    {
        UIControl* container = control->GetParent();
        sharedLayouter->ApplyLayoutNonRecursive(container);

        for (UILayoutSystemListener* listener : listeners)
        {
            listener->OnControlLayouted(container);
        }
    }
}

void UILayoutSystem::ManualApplyLayout(UIControl* control)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::UI_LAYOUT_SYSTEM);

    Layouter localLayouter;
    localLayouter.SetRtl(isRtl);
    localLayouter.ApplyLayout(control);
}

bool UILayoutSystem::IsAutoupdatesEnabled() const
{
    return autoupdatesEnabled;
}

void UILayoutSystem::SetAutoupdatesEnabled(bool enabled)
{
    autoupdatesEnabled = enabled;
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
    while (result->GetParent() != nullptr && result->GetComponentCount(UIComponent::LAYOUT_ISOLATION_COMPONENT) == 0)
    {
        UISizePolicyComponent* sizePolicy = result->GetParent()->GetComponent<UISizePolicyComponent>();
        if ((sizePolicy != nullptr && (sizePolicy->IsDependsOnChildren(Vector2::AXIS_X) || sizePolicy->IsDependsOnChildren(Vector2::AXIS_Y))) ||
            result->GetParent()->GetComponent(UIComponent::LAYOUT_SOURCE_RECT_COMPONENT) != nullptr)
        {
            result = result->GetParent();
        }
        else
        {
            break;
        }
    }

    if (result->GetParent() != nullptr && result->GetComponentCount(UIComponent::LAYOUT_ISOLATION_COMPONENT) == 0)
    {
        result = result->GetParent();
    }

    return result;
}

bool UILayoutSystem::HaveToLayoutAfterReorder(const UIControl* control) const
{
    static const uint64 sensitiveComponents = MAKE_COMPONENT_MASK(UIComponent::LINEAR_LAYOUT_COMPONENT) |
    MAKE_COMPONENT_MASK(UIComponent::FLOW_LAYOUT_COMPONENT);
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

bool UILayoutSystem::HaveToLayoutAfterReposition(const UIControl* control) const
{
    const UIControl* parent = control->GetParent();
    if (parent == nullptr)
    {
        return false;
    }

    if ((control->GetAvailableComponentFlags() & MAKE_COMPONENT_MASK(UIComponent::ANCHOR_COMPONENT)) != 0)
    {
        return true;
    }

    static const uint64 parentComponents = MAKE_COMPONENT_MASK(UIComponent::LINEAR_LAYOUT_COMPONENT) |
    MAKE_COMPONENT_MASK(UIComponent::FLOW_LAYOUT_COMPONENT);
    if ((parent->GetAvailableComponentFlags() & parentComponents) != 0)
    {
        return true;
    }

    return false;
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
