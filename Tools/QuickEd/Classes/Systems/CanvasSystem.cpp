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
#include "Document.h"
#include "UI/UIControl.h"
#include "Base/BaseTypes.h"
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/Shader.h"

using namespace DAVA;

class CheckeredCanvas : public DAVA::UIControl
{
public:
    CheckeredCanvas();
private:
    ~CheckeredCanvas() override = default;
    void Draw(const DAVA::UIGeometricData &geometricData) override;
};

CheckeredCanvas::CheckeredCanvas()
    : UIControl()
{
    GetBackground()->SetSprite("~res:/Gfx/CheckeredBg", 0);
    GetBackground()->SetDrawType(UIControlBackground::DRAW_TILED);
    GetBackground()->SetShader(SafeRetain(RenderSystem2D::TEXTURE_MUL_FLAT_COLOR));
}

void CheckeredCanvas::Draw(const UIGeometricData &geometricData)
{
    float32 invScale = 1.0f / geometricData.scale.x;
    UIGeometricData unscaledGd;
    unscaledGd.scale = Vector2(invScale, invScale);
    unscaledGd.size = geometricData.size * geometricData.scale.x;
    unscaledGd.AddGeometricData(geometricData);
    GetBackground()->Draw(unscaledGd);
}

CanvasSystem::CanvasSystem(Document *parent)
    : document(parent)
    , canvas(new UIControl())
{
}

void CanvasSystem::Attach(DAVA::UIControl* root)
{
    attachedRoot = root;
    attachedRoot->AddControl(canvas);
    SelectionWasChanged(SelectedControls(), SelectedControls());
}

void CanvasSystem::SelectionWasChanged(const SelectedControls& selected, const SelectedControls& deselected)
{
    UniteNodes(selected, selectedControls);
    SubstractNodes(deselected, selectedControls);
    SelectedNodes rootControls;
    if (selectedControls.empty())
    {
        auto controlsNode = document->GetPackage()->GetPackageControlsNode();
        for (int index = 0; index < controlsNode->GetCount(); ++index)
        {
            rootControls.insert(controlsNode->Get(index));
        }
    }
    else
    {
        for (auto &node : selectedControls)
        {
            if (nullptr != node->GetControl())
            {
                PackageBaseNode *root = node;
                while (root->GetParent() && root->GetParent()->GetControl())
                {
                    root = root->GetParent();
                }
                rootControls.insert(root);
            }
        }
    }
    SetRootControls(rootControls);
}

void CanvasSystem::SetRootControls(const SelectedNodes& controls)
{
    canvas->RemoveAllControls();
    for(auto node : controls)
    {
        ScopedPtr<CheckeredCanvas> checkeredCanvas(new CheckeredCanvas());
        checkeredCanvas->AddControl(node->GetControl());
        checkeredCanvas->SetSize(node->GetControl()->GetSize());
        canvas->AddControl(checkeredCanvas);
    }
    LayoutCanvas();
}

void CanvasSystem::LayoutCanvas()
{
    float32 maxWidth = 0.0f;
    float32 totalHeight = 0.0f;
    for (auto control : canvas->GetChildren())
    {
        maxWidth = Max(maxWidth, control->GetSize().x);
        totalHeight += control->GetSize().y;
    }
    Vector2 size(maxWidth, totalHeight);
    attachedRoot->SetSize(size);
    canvas->SetSize(size);
    float32 curY = 0.0f;
    for (auto control : canvas->GetChildren())
    {
        Rect rect = control->GetRect();
        rect.y = curY;
        rect.x = (maxWidth - rect.dx) / 2.0f;
        control->SetRect(rect);

        curY += rect.dy + 5;
    }
}
