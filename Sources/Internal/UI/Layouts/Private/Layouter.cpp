#include "Layouter.h"
#include "UI/Layouts/LayoutFormula.h"
#include "UI/Layouts/Private/AnchorLayoutAlgorithm.h"
#include "UI/Layouts/Private/FlowLayoutAlgorithm.h"
#include "UI/Layouts/Private/LinearLayoutAlgorithm.h"
#include "UI/Layouts/Private/SizeMeasuringAlgorithm.h"
#include "UI/Layouts/UIAnchorComponent.h"
#include "UI/Layouts/UIFlowLayoutComponent.h"
#include "UI/Layouts/UILinearLayoutComponent.h"
#include "UI/Layouts/UISizePolicyComponent.h"
#include "UI/UIControl.h"

namespace DAVA
{
void Layouter::ApplyLayout(UIControl* control)
{
    CollectControls(control, true);

    ProcessAxis(Vector2::AXIS_X, true);
    ProcessAxis(Vector2::AXIS_Y, true);

    ApplySizesAndPositions();

    layoutData.clear();
}

void Layouter::ApplyLayoutNonRecursive(UIControl* control)
{
    CollectControls(control, false);

    ProcessAxis(Vector2::AXIS_X, false);
    ProcessAxis(Vector2::AXIS_Y, false);

    ApplyPositions();

    layoutData.clear();
}

void Layouter::CollectControls(UIControl* control, bool recursive)
{
    layoutData.clear();
    layoutData.emplace_back(ControlLayoutData(control));
    CollectControlChildren(control, 0, recursive);
}

void Layouter::CollectControlChildren(UIControl* control, int32 parentIndex, bool recursive)
{
    int32 index = static_cast<int32>(layoutData.size());
    const List<UIControl*>& children = control->GetChildren();

    int32 childrenCount = 0;

    for (UIControl* child : children)
    {
        if (child->GetComponentCount(UIComponent::LAYOUT_ISOLATION_COMPONENT) == 0)
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
            if (child->GetComponentCount(UIComponent::LAYOUT_ISOLATION_COMPONENT) == 0)
            {
                CollectControlChildren(child, index, recursive);
                index++;
            }
        }
    }
}

void Layouter::ProcessAxis(Vector2::eAxis axis, bool processSizes)
{
    if (processSizes)
    {
        DoMeasurePhase(axis);
    }
    DoLayoutPhase(axis);
}

void Layouter::DoMeasurePhase(Vector2::eAxis axis)
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

void Layouter::DoLayoutPhase(Vector2::eAxis axis)
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
                    if (onFormulaRemoved)
                    {
                        onFormulaRemoved(sizePolicy->GetControl(), axis, formula);
                    }
                    sizePolicy->RemoveFormula(axis);
                }
                else
                {
                    if (onFormulaProcessed)
                    {
                        onFormulaProcessed(sizePolicy->GetControl(), axis, formula);
                    }
                }
            }
        }
    }
}

void Layouter::ApplySizesAndPositions()
{
    for (ControlLayoutData& data : layoutData)
    {
        data.ApplyLayoutToControl();
    }
}

void Layouter::ApplyPositions()
{
    for (ControlLayoutData& data : layoutData)
    {
        data.ApplyOnlyPositionLayoutToControl();
    }
}

void Layouter::SetRtl(bool rtl)
{
    isRtl = rtl;
}
}