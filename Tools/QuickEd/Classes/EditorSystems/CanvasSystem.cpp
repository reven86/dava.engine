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
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/Shader.h"
#include "Model/ControlProperties/PropertyListener.h"
#include "Model/ControlProperties/RootProperty.h"

using namespace DAVA;

class GridControl : public UIControl
{
public:
    GridControl(CanvasSystem* canvasSystem);
    ~GridControl() override = default;
    void Init(UIControl* control);
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* propert);
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from);
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* /*destination*/, int /*index*/);
    void AdjustToNestedControl();

private:
    void CalculateTotalRect(UIControl* control, Rect& totalRect, Vector2& rootControlPosition);
    void UpdateSprite();
    void Draw(const UIGeometricData& geometricData) override;
    ScopedPtr<UIControl> counterPoiseControl;
    ScopedPtr<UIControl> positionHolderControl;
    CanvasSystem* canvasSystem = nullptr;
    UIControl* nestedControl = nullptr;
};

GridControl::GridControl(CanvasSystem* canvasSystem_)
    : UIControl()
    , counterPoiseControl(new UIControl())
    , positionHolderControl(new UIControl())
    , canvasSystem(canvasSystem_)
{
}

void GridControl::Init(UIControl* control)
{
    nestedControl = control;
    String name = control->GetName();
    name = name.empty() ? "unnamed" : name;
    SetName("Grid control of " + name);
    counterPoiseControl->SetName("counterPoise of " + name);
    positionHolderControl->SetName("Position holder of " + name);
    AddControl(positionHolderControl);
    positionHolderControl->AddControl(counterPoiseControl);
    counterPoiseControl->AddControl(control);

    background->SetDrawType(UIControlBackground::DRAW_TILED);
    background->SetShader(RenderSystem2D::TEXTURE_MUL_FLAT_COLOR);

    counterPoiseControl->SetSize(control->GetSize() * control->GetScale());
    counterPoiseControl->SetPosition(-control->GetPosition());
    counterPoiseControl->SetPivot(-control->GetPivot());
    counterPoiseControl->SetAngle(-control->GetAngle());
    UpdateSprite();
    AdjustToNestedControl();
}

void GridControl::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    const String& name = property->GetName();
    if (node->GetControl() == nestedControl)
    {
        if (name == "Position")
        {
            Vector2 position = property->GetValue().AsVector2();
            counterPoiseControl->SetPosition(-position);
            UpdateSprite();
        }
        else if (name == "Pivot")
        {
            Vector2 pivot = property->GetValue().AsVector2();
            counterPoiseControl->SetPivot(-pivot);
            UpdateSprite();
        }
        else if (name == "Angle")
        {
            float32 angle = property->GetValue().AsFloat();
            counterPoiseControl->SetAngle(-DegToRad(angle));
            UpdateSprite();
        }
    }
    if (name == "Angle" || name == "Size" || name == "Scale" || name == "Position" || name == "Pivot")
    {
        UIControl* control = node->GetControl();
        PackageBaseNode* parent = node;
        while (nullptr != control && control != nestedControl)
        {
            parent = parent->GetParent();
            control = nullptr != parent ? parent->GetControl() : nullptr;
        }
        if (control == nestedControl)
        {
            AdjustToNestedControl();
            canvasSystem->LayoutCanvas();
        }
    }
}

void CalculateTotalRectImpl(UIControl* control, Rect& totalRect, Vector2& rootControlPosition, const UIGeometricData& gd)
{
    UIGeometricData tempGeometricData;
    tempGeometricData.position = control->GetPosition();
    tempGeometricData.size = control->GetSize();
    tempGeometricData.pivotPoint = control->GetPivotPoint();
    tempGeometricData.scale = control->GetScale();
    tempGeometricData.angle = control->GetAngle();
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

void GridControl::CalculateTotalRect(UIControl* control, Rect& totalRect, Vector2& rootControlPosition)
{
    rootControlPosition.SetZero();
    UIGeometricData gd = control->GetGeometricData();
    gd.position.SetZero();

    Vector2 scale = canvasSystem->systemManager->GetScalableControl()->GetScale();
    if (scale.x != 0.0f || scale.y != 0.0f)
    {
        gd.scale /= scale;

        totalRect = gd.GetAABBox();

        for (const auto& child : control->GetChildren())
        {
            CalculateTotalRectImpl(child, totalRect, rootControlPosition, gd);
        }
    }
}

void GridControl::AdjustToNestedControl()
{
    Rect rect;
    Vector2 pos;
    CalculateTotalRect(nestedControl, rect, pos);
    Vector2 size = rect.GetSize();
    positionHolderControl->SetPosition(pos);
    SetSize(size);
    UpdateSprite();
}

void GridControl::ControlWasRemoved(ControlNode* node, ControlsContainerNode* from)
{
    //check that we adjust removed node
    if (node->GetControl() == nestedControl)
    {
        return;
    }
    UIControl* control = from->GetControl();
    PackageBaseNode* parent = from;
    while (nullptr != control && control != nestedControl)
    {
        parent = parent->GetParent();
        control = nullptr != parent ? parent->GetControl() : nullptr;
    }
    if (control == nestedControl)
    {
        AdjustToNestedControl();
        canvasSystem->LayoutCanvas();
    }
}

void GridControl::ControlWasAdded(ControlNode* /*node*/, ControlsContainerNode* destination, int /*index*/)
{
    UIControl* control = destination->GetControl();
    PackageBaseNode* parent = destination;
    while (nullptr != control && control != nestedControl)
    {
        parent = parent->GetParent();
        control = nullptr != parent ? parent->GetControl() : nullptr;
    }
    if (control == nestedControl)
    {
        AdjustToNestedControl();
        canvasSystem->LayoutCanvas();
    }
}

void GridControl::UpdateSprite()
{
    /*bool transformed = !counterPoiseControl->GetPosition().IsZero() || !counterPoiseControl->GetPivot().IsZero() || counterPoiseControl->GetAngle() != 0;
    if (transformed)
    {
        background->SetSprite("~res:/Gfx/CheckeredBg2", 0);
    }
    else*/ //use it to indicate that root control is in invalid state
    {
        background->SetSprite("~res:/Gfx/CheckeredBg", 0);
    }
}

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

CanvasSystem::CanvasSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
    , controlsCanvas(new UIControl())
{
    systemManager->GetPackage()->AddListener(this);

    controlsCanvas->SetName("controls canvas");

    auto controlsNode = systemManager->GetPackage()->GetPackageControlsNode();
    for (int index = 0; index < controlsNode->GetCount(); ++index)
    {
        CreateAndInsertGrid(controlsNode->Get(index), index);
    }
}

CanvasSystem::~CanvasSystem()
{
    systemManager->GetPackage()->RemoveListener(this);
}

void CanvasSystem::OnActivated()
{
    systemManager->GetScalableControl()->AddControl(controlsCanvas);

    LayoutCanvas();
}

void CanvasSystem::OnDeactivated()
{
    controlsCanvas->RemoveFromParent();
}

void CanvasSystem::ControlWasRemoved(ControlNode* node, ControlsContainerNode* from)
{
    const PackageControlsNode* packageControlsNode = systemManager->GetPackage()->GetPackageControlsNode();
    if (packageControlsNode == from)
    {
        UIControl* removedControl = node->GetControl();
        UIControl* grid = removedControl->GetParent()->GetParent()->GetParent(); //grid<-positionHolder<-counterPoise<-control
        gridControls.remove(DynamicTypeCheck<GridControl*>(grid));
        controlsCanvas->RemoveControl(grid);
    }
    for (GridControl* grid : gridControls)
    {
        grid->ControlWasRemoved(node, from);
    }
}

void CanvasSystem::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index)
{
    auto packageControlsNode = systemManager->GetPackage()->GetPackageControlsNode();
    if (packageControlsNode == node->GetParent())
    {
        CreateAndInsertGrid(node, index);
    }
    for (GridControl* grid : gridControls)
    {
        grid->ControlWasAdded(node, destination, index);
    }
}

void CanvasSystem::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    for (GridControl* grid : gridControls)
    {
        grid->ControlPropertyWasChanged(node, property);
    }
}

void CanvasSystem::CreateAndInsertGrid(ControlNode* controlNode, int pos)
{
    ScopedPtr<GridControl> gridControl(new GridControl(this));
    auto iter = gridControls.begin();
    std::advance(iter, pos);
    gridControls.insert(iter, gridControl.get());
    UIControl* control = controlNode->GetControl();
    gridControl->Init(control);
    if (pos >= controlsCanvas->GetChildren().size())
    {
        controlsCanvas->AddControl(gridControl);
    }
    else
    {
        auto iter = controlsCanvas->GetChildren().begin();
        std::advance(iter, pos);
        controlsCanvas->InsertChildBelow(gridControl, *iter);
    }
}

void CanvasSystem::LayoutCanvas()
{
    float32 maxWidth = 0.0f;
    float32 totalHeight = 0.0f;
    const int spacing = 5;
    int childrenCount = gridControls.size();
    if (childrenCount > 1)
    {
        totalHeight += spacing * (childrenCount - 1);
    }
    for (const GridControl* grid : gridControls)
    {
        maxWidth = Max(maxWidth, grid->GetSize().dx);
        totalHeight += grid->GetSize().dy;
    }

    float32 curY = 0.0f;
    for (GridControl* grid : gridControls)
    {
        Rect rect = grid->GetRect();
        rect.y = curY;
        rect.x = (maxWidth - rect.dx) / 2.0f;
        grid->SetRect(rect);
        curY += rect.dy + spacing;
    }
    Vector2 size(maxWidth, totalHeight);
    systemManager->GetScalableControl()->SetSize(size);
    systemManager->CanvasSizeChanged.Emit();
}
