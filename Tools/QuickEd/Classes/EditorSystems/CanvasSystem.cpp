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

class GridCanvas : public UIControl, private PropertyListener
{
public:
    GridCanvas(RootProperty* property);
    void PropertyChanged(AbstractProperty* property) override;

private:
    void Draw(const UIGeometricData& geometricData) override;
    AbstractProperty* sizeProperty = nullptr;
    AbstractProperty* positionProperty = nullptr;
};

GridCanvas::GridCanvas(RootProperty* property)
    : UIControl()
{
    property->AddListener(this);
    sizeProperty = property->FindPropertyByName("Size");
    positionProperty = property->FindPropertyByName("Position");
    DVASSERT(nullptr != sizeProperty);
    DVASSERT(nullptr != positionProperty);
    SetName("GridCanvas");
    background->SetSprite("~res:/Gfx/CheckeredBg", 0);
    background->SetDrawType(UIControlBackground::DRAW_TILED);
    background->SetShader(RenderSystem2D::TEXTURE_MUL_FLAT_COLOR);
}

void GridCanvas::PropertyChanged(AbstractProperty* property)
{
    if (property == sizeProperty)
    {
        SetSize(sizeProperty->GetValue().AsVector2());
    }
}

void GridCanvas::Draw(const UIGeometricData& geometricData)
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
    controlsCanvas->SetName("controls canvas");
    systemManager->GetPackage()->AddListener(this);
}

void CanvasSystem::OnActivated()
{
    systemManager->GetScalableControl()->AddControl(controlsCanvas);

    auto controlsNode = systemManager->GetPackage()->GetPackageControlsNode();
    for (int index = 0; index < controlsNode->GetCount(); ++index)
    {
        AddRootControl(controlsNode->Get(index));
    }
    LayoutCanvas();
}

void CanvasSystem::OnDeactivated()
{
    controlsCanvas->RemoveFromParent();
}

void CanvasSystem::ControlWillBeRemoved(::ControlNode* node, ControlsContainerNode* from)
{
    if (nullptr == node->GetParent())
    {
        UIControl* removedControl = node->GetControl()->GetParent();
        controlsCanvas->RemoveControl(removedControl);
        LayoutCanvas();
    }
}

void CanvasSystem::ControlWasAdded(ControlNode* node, ControlsContainerNode* /*destination*/, int /*index*/)
{
    auto packageControlsNode = systemManager->GetPackage()->GetPackageControlsNode();
    if (packageControlsNode == node->GetParent())
    {
        AddRootControl(node);
        LayoutCanvas();
    }
}

void CanvasSystem::SetRootControls(const Vector<ControlNode*>& controls)
{
    for (auto node : controls)
    {
        AddRootControl(node);
    }
}

void CanvasSystem::AddRootControl(ControlNode* controlNode)
{
    ScopedPtr<GridCanvas> gridControl(new GridCanvas(controlNode->GetRootProperty()));
    UIControl* control = controlNode->GetControl();
    gridControl->SetSize(control->GetSize());
    gridControl->AddControl(control);
    controlsCanvas->AddControl(gridControl);
}

void CanvasSystem::LayoutCanvas()
{
    float32 maxWidth = 0.0f;
    float32 totalHeight = 0.0f;
    const int spacing = 5;
    int childrenCount = controlsCanvas->GetChildren().size();
    if (childrenCount > 1)
    {
        totalHeight += spacing * (childrenCount - 1);
    }
    for (auto control : controlsCanvas->GetChildren())
    {
        maxWidth = Max(maxWidth, control->GetSize().x);
        totalHeight += control->GetSize().y;
    }

    float32 curY = 0.0f;
    for (auto control : controlsCanvas->GetChildren())
    {
        Rect rect = control->GetRect();
        rect.y = curY;
        rect.x = (maxWidth - rect.dx) / 2.0f;
        control->SetRect(rect);
        curY += rect.dy + spacing;
    }
    Vector2 size(maxWidth, totalHeight);
    controlsCanvas->SetSize(size);
    systemManager->GetRootControl()->SetSize(size);
    systemManager->CanvasSizeChanged.Emit();
}
