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

#include "HUDSystem.h"

#include "UI/UIControl.h"
#include "Base/BaseTypes.h"
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Render/RenderState.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

using namespace DAVA;
class FrameControl : public DAVA::UIControl
{
    void Draw(const DAVA::UIGeometricData &geometricData) override
    {
        Color oldColor = RenderManager::Instance()->GetColor();
        RenderManager::Instance()->SetColor(Color(1.0f, 0.0f, 0.0f, 1.f));
        RenderHelper::Instance()->DrawRect(Rect(geometricData.position, geometricData.size * GetGeometricData().scale), RenderState::RENDERSTATE_2D_BLEND);
        RenderManager::Instance()->SetColor(oldColor);
    }
};

class FrameRectControl : public DAVA::UIControl
{
    void Draw(const DAVA::UIGeometricData &geometricData) override
    {
        Color oldColor = RenderManager::Instance()->GetColor();
        RenderManager::Instance()->SetColor(0.0f, 1.0f, 0.0f, 1.f);
        Rect trueRect(geometricData.position, geometricData.size * GetGeometricData().scale);
        Rect drawRect(Vector2(0, 0), geometricData.size);
        drawRect.SetCenter(trueRect.GetCenter());
        RenderHelper::Instance()->FillRect(drawRect, RenderState::RENDERSTATE_2D_BLEND);
        RenderManager::Instance()->SetColor(oldColor);
    }
};

HUDSystem::HUDSystem()
{
    hudControl = new UIControl();
}

HUDSystem::~HUDSystem()
{
    hudControl->Release();
}

void HUDSystem::Attach(DAVA::UIControl* root)
{
    root->AddControl(hudControl);
}

void HUDSystem::SelectionWasChanged(const SelectedControls& selected, const SelectedControls& deselected)
{
    hudControl->RemoveAllControls();
    for (auto control : selected)
    {
        ScopedPtr<UIControl> frame(new FrameControl());
        frame->SetRect(control->GetControl()->GetRect());
        hudControl->AddControl(frame);
        ScopedPtr<UIControl> r(new FrameRectControl());
        Rect rect(Vector2(0, 0), Vector2(10, 10));
        rect.SetCenter(control->GetControl()->GetPosition());
        r->SetRect(rect);
        hudControl->AddControl(r);
    }
}

