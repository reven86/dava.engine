/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "CanvasSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "UI/UIControl.h"
#include "Base/BaseTypes.h"
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/ControlProperties/RootProperty.h"

using namespace DAVA;

namespace
{
class GridControl : public UIControl
{
public:
    GridControl() = default;
    ~GridControl() override = default;

private:
    void Draw(const UIGeometricData& geometricData) override;
};

void GridControl::Draw(const UIGeometricData& geometricData)
{
    if (0.0f != geometricData.scale.x)
    {
        float32 invScale = 1.0f / geometricData.scale.x;
        UIGeometricData unscaledGd;
        unscaledGd.scale = Vector2(invScale, invScale);
        unscaledGd.size = geometricData.size * geometricData.scale.x;
        unscaledGd.AddGeometricData(geometricData);
        GetBackground()->Draw(unscaledGd);
    }
}

} //unnamed namespe

class BackgroundController final
{
public:
    BackgroundController(UIControl* nestedControl);
    ~BackgroundController() = default;
    UIControl* GetGridControl();
    bool IsNestedControl(const UIControl* control) const;
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* propert);
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from);
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* /*destination*/, int /*index*/);
    void UpdateCounterpoise();
    void AdjustToNestedControl();
    Signal<> ContentSizeChanged;

private:
    void CalculateTotalRect(Rect& totalRect, Vector2& rootControlPosition);
    void FitGridIfParentIsNested(PackageBaseNode* node);
    RefPtr<UIControl> gridControl;
    RefPtr<UIControl> counterpoiseControl;
    RefPtr<UIControl> positionHolderControl;
    UIControl* nestedControl = nullptr;
};

BackgroundController::BackgroundController(UIControl* nestedControl_)
    : gridControl(new GridControl())
    , counterpoiseControl(new UIControl())
    , positionHolderControl(new UIControl())
    , nestedControl(nestedControl_)
{
    DVASSERT(nullptr != nestedControl);
    String name = nestedControl->GetName();
    name = name.empty() ? "unnamed" : name;
    gridControl->SetName("Grid control of " + name);
    counterpoiseControl->SetName("counterpoise of " + name);
    positionHolderControl->SetName("Position holder of " + name);
    gridControl->AddControl(positionHolderControl.Get());
    positionHolderControl->AddControl(counterpoiseControl.Get());
    counterpoiseControl->AddControl(nestedControl);

    gridControl->GetBackground()->SetDrawType(UIControlBackground::DRAW_TILED);
    gridControl->GetBackground()->SetSprite("~res:/Gfx/GreyGrid", 0);
}

UIControl* BackgroundController::GetGridControl()
{
    return gridControl.Get();
}

bool BackgroundController::IsNestedControl(const DAVA::UIControl* control) const
{
    return control == nestedControl;
}

void BackgroundController::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    const String& name = property->GetName();
    if (node->GetControl() == nestedControl)
    {
        if (name == "Angle" || name == "Size" || name == "Scale" || name == "Position" || name == "Pivot" || name == "Visible")
        {
            UpdateCounterpoise();
        }
    }
    if (name == "Angle" || name == "Size" || name == "Scale" || name == "Position" || name == "Pivot" || name == "Visible")
    {
        FitGridIfParentIsNested(node);
    }
}

namespace
{
void CalculateTotalRectImpl(UIControl* control, Rect& totalRect, Vector2& rootControlPosition, const UIGeometricData& gd)
    {
        if (!control->GetSystemVisible())
        {
            return;
        }
        UIGeometricData tempGeometricData = control->GetLocalGeometricData();
        tempGeometricData.AddGeometricData(gd);
        Rect box = tempGeometricData.GetAABBox();

        if (totalRect.x > box.x)
        {
            float32 delta = totalRect.x - box.x;
            rootControlPosition.x += delta;
            totalRect.dx += delta;
            totalRect.x = box.x;
        }
        if (totalRect.y > box.y)
        {
            float32 delta = totalRect.y - box.y;
            rootControlPosition.y += delta;
            totalRect.dy += delta;
            totalRect.y = box.y;
        }
        if (totalRect.x + totalRect.dx < box.x + box.dx)
        {
            float32 nextRight = box.x + box.dx;
            totalRect.dx = nextRight - totalRect.x;
        }
        if (totalRect.y + totalRect.dy < box.y + box.dy)
        {
            float32 nextBottom = box.y + box.dy;
            totalRect.dy = nextBottom - totalRect.y;
        }

        for (const auto& child : control->GetChildren())
        {
            CalculateTotalRectImpl(child, totalRect, rootControlPosition, tempGeometricData);
        }
    }
    } //unnamed namespace

void BackgroundController::CalculateTotalRect(Rect& totalRect, Vector2& rootControlPosition)
{
    rootControlPosition.SetZero();
    UIGeometricData gd = nestedControl->GetGeometricData();
    gd.position.SetZero();
    UIControl* scalableControl = gridControl->GetParent()->GetParent();
    DVASSERT_MSG(nullptr != scalableControl, "grid update without being attached to screen");
    gd.scale /= scalableControl->GetScale(); //grid->controlCanvas->scalableControl
    if (gd.scale.x != 0.0f || gd.scale.y != 0.0f)
    {
        totalRect = gd.GetAABBox();

        for (const auto& child : nestedControl->GetChildren())
        {
            CalculateTotalRectImpl(child, totalRect, rootControlPosition, gd);
        }
    }
}

void BackgroundController::AdjustToNestedControl()
{
    Rect rect;
    Vector2 pos;
    CalculateTotalRect(rect, pos);
    Vector2 size = rect.GetSize();
    positionHolderControl->SetPosition(pos);
    gridControl->SetSize(size);
    ContentSizeChanged.Emit();
}

void BackgroundController::ControlWasRemoved(ControlNode* node, ControlsContainerNode* from)
{
    //check that we adjust removed node
    if (node->GetControl() == nestedControl)
    {
        return;
    }
    FitGridIfParentIsNested(from);
}

void BackgroundController::ControlWasAdded(ControlNode* /*node*/, ControlsContainerNode* destination, int /*index*/)
{
    FitGridIfParentIsNested(destination);
}

void BackgroundController::UpdateCounterpoise()
{
    UIGeometricData gd = nestedControl->GetLocalGeometricData();
    Vector2 invScale;
    invScale.x = gd.scale.x != 0.0f ? 1.0f / gd.scale.x : 0.0f;
    invScale.y = gd.scale.y != 0.0f ? 1.0f / gd.scale.y : 0.0f;
    counterpoiseControl->SetScale(invScale);
    counterpoiseControl->SetSize(gd.size);
    gd.cosA = cosf(gd.angle);
    gd.sinA = sinf(gd.angle);
    counterpoiseControl->SetAngle(-gd.angle);
    Vector2 pos(gd.position * invScale);
    Vector2 angeledPosition(pos.x * gd.cosA + pos.y * gd.sinA,
                            pos.x * -gd.sinA + pos.y * gd.cosA);

    counterpoiseControl->SetPosition(-angeledPosition + gd.pivotPoint);
}

void BackgroundController::FitGridIfParentIsNested(PackageBaseNode* node)
{
    PackageBaseNode* parent = node;
    while (nullptr != parent)
    {
        if (parent->GetControl() == nestedControl) //we change child in the nested control
        {
            AdjustToNestedControl();
            return;
        }
        parent = parent->GetParent();
    }
}

CanvasSystem::CanvasSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
    , controlsCanvas(new UIControl())
{
    systemManager->GetPackage()->AddListener(this);

    controlsCanvas->SetName("controls canvas");

    systemManager->EditingRootControlsChanged.Connect(this, &CanvasSystem::OnRootContolsChanged);
}

CanvasSystem::~CanvasSystem()
{
    PackageNode* package = systemManager->GetPackage();
    if (nullptr != package)
    {
        systemManager->GetPackage()->RemoveListener(this);
    }
    systemManager->GetScalableControl()->RemoveControl(controlsCanvas.Get());
}

void CanvasSystem::OnActivated()
{
    systemManager->GetScalableControl()->AddControl(controlsCanvas.Get());
    for (auto& iter : gridControls)
    {
        iter->AdjustToNestedControl();
    }
}

void CanvasSystem::OnDeactivated()
{
    systemManager->GetScalableControl()->RemoveControl(controlsCanvas.Get());
    systemManager->GetScalableControl()->SetSize(Vector2());
    systemManager->CanvasSizeChanged.Emit();
}

void CanvasSystem::ControlWasRemoved(ControlNode* node, ControlsContainerNode* from)
{
    if (nullptr == controlsCanvas->GetParent())
    {
        return;
    }
    for (auto& iter : gridControls)
    {
        iter->ControlWasRemoved(node, from);
    }
}

void CanvasSystem::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index)
{
    if (nullptr == controlsCanvas->GetParent())
    {
        return;
    }
    for (auto& iter : gridControls)
    {
        iter->ControlWasAdded(node, destination, index);
    }
}

void CanvasSystem::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    if (nullptr == controlsCanvas->GetParent())
    {
        return;
    }
    for (auto& iter : gridControls)
    {
        iter->ControlPropertyWasChanged(node, property);
    }
}

BackgroundController* CanvasSystem::CreateControlBackground(PackageBaseNode* node)
{
    BackgroundController* backgroundController(new BackgroundController(node->GetControl()));
    backgroundController->ContentSizeChanged.Connect(this, &CanvasSystem::LayoutCanvas);
    gridControls.emplace_back(backgroundController);
    return backgroundController;
}

void CanvasSystem::AddBackgroundControllerToCanvas(BackgroundController* backgroundController, size_t pos)
{
    UIControl* grid = backgroundController->GetGridControl();
    if (pos >= controlsCanvas->GetChildren().size())
    {
        controlsCanvas->AddControl(grid);
    }
    else
    {
        auto iterToInsert = controlsCanvas->GetChildren().begin();
        std::advance(iterToInsert, pos);
        controlsCanvas->InsertChildBelow(grid, *iterToInsert);
    }
    backgroundController->UpdateCounterpoise();
    backgroundController->AdjustToNestedControl();
}

void CanvasSystem::LayoutCanvas()
{
    float32 maxWidth = 0.0f;
    float32 totalHeight = 0.0f;
    const int spacing = 5;
    const List<UIControl*>& children = controlsCanvas->GetChildren();
    int childrenCount = children.size();
    if (childrenCount > 1)
    {
        totalHeight += spacing * (childrenCount - 1);
    }
    for (const UIControl* control : children)
    {
        maxWidth = Max(maxWidth, control->GetSize().dx);
        totalHeight += control->GetSize().dy;
    }

    float32 curY = 0.0f;
    for (UIControl* child : children)
    {
        Rect rect = child->GetRect();
        rect.y = curY;
        rect.x = (maxWidth - rect.dx) / 2.0f;
        child->SetRect(rect);
        curY += rect.dy + spacing;
    }
    Vector2 size(maxWidth, totalHeight);
    systemManager->GetScalableControl()->SetSize(size);
    systemManager->CanvasSizeChanged.Emit();
}

void CanvasSystem::OnRootContolsChanged(const EditorSystemsManager::SortedPackageBaseNodeSet& rootControls_)
{
    DAVA::Set<PackageBaseNode*> sortedRootControls(rootControls_.begin(), rootControls_.end());
    DAVA::Set<PackageBaseNode*> newNodes;
    DAVA::Set<PackageBaseNode*> deletedNodes;
    if (!rootControls.empty())
    {
        std::set_difference(rootControls.begin(), rootControls.end(), sortedRootControls.begin(), sortedRootControls.end(), std::inserter(deletedNodes, deletedNodes.end()));
    }
    if (!sortedRootControls.empty())
    {
        std::set_difference(sortedRootControls.begin(), sortedRootControls.end(), rootControls.begin(), rootControls.end(), std::inserter(newNodes, newNodes.end()));
    }
    rootControls = sortedRootControls;

    for (auto iter = deletedNodes.begin(); iter != deletedNodes.end(); ++iter)
    {
        PackageBaseNode* node = *iter;
        UIControl* control = node->GetControl();
        auto findIt = std::find_if(gridControls.begin(), gridControls.end(), [control](std::unique_ptr<BackgroundController>& gridIter) {
            return gridIter->IsNestedControl(control);
        });
        DVASSERT(findIt != gridControls.end());
        controlsCanvas->RemoveControl(findIt->get()->GetGridControl());
        gridControls.erase(findIt);
    }
    DVASSERT(rootControls_.size() == rootControls.size());
    for (auto iter = rootControls_.begin(); iter != rootControls_.end(); ++iter)
    {
        PackageBaseNode* node = *iter;
        if (newNodes.find(node) == newNodes.end())
        {
            continue;
        }
        UIControl* control = node->GetControl();
        DVASSERT(std::find_if(gridControls.begin(), gridControls.end(), [control](std::unique_ptr<BackgroundController>& gridIter) {
                     return gridIter->IsNestedControl(control);
                 }) == gridControls.end());
        BackgroundController* backgroundController = CreateControlBackground(node);
        AddBackgroundControllerToCanvas(backgroundController, std::distance(rootControls_.begin(), iter));
    }
    LayoutCanvas();
}