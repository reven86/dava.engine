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
#include "UI/UIEvent.h"
#include "Base/BaseTypes.h"
#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"
#include "UI/Layouts/UISizePolicyComponent.h"
#include "Render/RenderState.h"
#include "Render/RenderManager.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ImportedPackagesNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

using namespace DAVA;

namespace
{
const Vector2 PIVOT_CONTROL_SIZE(20.0f, 20.0f);
const Vector2 FRAME_RECT_SIZE(15.0f, 15.0f);
const Vector2 ROTATE_CONTROL_SIZE(20.0f, 20.0f);
const Array<HUDAreaInfo::eArea, 2> AreasToHide = {{HUDAreaInfo::PIVOT_POINT_AREA, HUDAreaInfo::ROTATE_AREA}};
}

class SelectionRect : public UIControl
{
public:
    enum
    {
        BORDER_TOP,
        BORDER_BOTTOM,
        BORDER_LEFT,
        BORDER_RIGHT,
        BORDERS_COUNT
    };
    explicit SelectionRect()
        : UIControl()
    {
        SetName("Selection Rect");
        for (uint32 i = 0; i < BORDERS_COUNT; ++i)
        {
            UIControl* control = new UIControl();
            control->SetName("border of selection rect");
            UIControlBackground* background = control->GetBackground();
            background->SetSprite("~res:/Gfx/HUDControls/BlackGrid", 0);
            background->SetDrawType(UIControlBackground::DRAW_TILED);

            borders.emplace_back(control);
        }
    }

private:
    void Draw(const UIGeometricData& geometricData) override
    {
        const Rect& rect = geometricData.GetUnrotatedRect();
        SetAbsoluteRect(rect);
        for (uint32 i = 0; i < BORDERS_COUNT; ++i)
        {
            if (firstInit)
            {
                AddControl(borders[i]);
            }
            Rect borderRect = CreateFrameBorderRect(i, rect);
            borders[i]->SetAbsoluteRect(borderRect);
        }
        firstInit = false;
        UIControl::Draw(geometricData);
    }

    Rect CreateFrameBorderRect(uint32 border, const Rect& frameRect) const
    {
        if (frameRect.GetSize().IsZero())
        {
            return Rect();
        }
        switch (border)
        {
        case BORDER_TOP:
            return Rect(frameRect.x, frameRect.y, frameRect.dx, 1.0f);
        case BORDER_BOTTOM:
            return Rect(frameRect.x, frameRect.y + frameRect.dy, frameRect.dx, 1.0f);
        case BORDER_LEFT:
            return Rect(frameRect.x, frameRect.y, 1.0f, frameRect.dy);
        case BORDER_RIGHT:
            return Rect(frameRect.x + frameRect.dx, frameRect.y, 1.0f, frameRect.dy);
        default:
            DVASSERT("!impossible value for frame control position");
            return Rect();
        }
    }
    bool firstInit = true;
    Vector<ScopedPtr<UIControl>> borders;
};

ControlContainer::ControlContainer(const HUDAreaInfo::eArea area_)
    : UIControl()
    , area(area_)
{
}

HUDAreaInfo::eArea ControlContainer::GetArea() const
{
    return area;
}

class HUDContainer : public ControlContainer
{
public:
    explicit HUDContainer(UIControl* container)
        : ControlContainer(HUDAreaInfo::NO_AREA)
        , control(container)
    {
        SetName("HudContainer of " + container->GetName());
    }
    void AddChild(ControlContainer* container)
    {
        AddControl(container);
        childs.push_back(container);
    }
    void InitFromGD(const UIGeometricData& geometricData) override
    {
        const Rect& ur = geometricData.GetUnrotatedRect();
        SetSize(ur.GetSize());
        SetPivot(control->GetPivot());
        SetAbsoluteRect(ur);
        SetAngle(geometricData.angle);
        for (auto child : childs)
        {
            child->InitFromGD(geometricData);
        }
    }

    void SystemDraw(const UIGeometricData& geometricData) override
    {
        InitFromGD(control->GetGeometricData());
        UIControl::SystemDraw(geometricData);
    }

private:
    UIControl* control = nullptr;
    Vector<ControlContainer*> childs;
};

class FrameControl : public ControlContainer
{
public:
    enum
    {
        BORDER_TOP,
        BORDER_BOTTOM,
        BORDER_LEFT,
        BORDER_RIGHT,
        BORDERS_COUNT
    };
    explicit FrameControl()
        : ControlContainer(HUDAreaInfo::FRAME_AREA)
    {
        SetName("Frame Control");
        for (uint32 i = 0; i < BORDERS_COUNT; ++i)
        {
            UIControl* control = new UIControl();
            control->SetName("border of frame");
            UIControlBackground* background = control->GetBackground();
            background->SetSprite("~res:/Gfx/HUDControls/BlackGrid", 0);
            background->SetDrawType(UIControlBackground::DRAW_TILED);
            borders.emplace_back(control);
        }
    }

private:
    void InitFromGD(const UIGeometricData& geometricData) override
    {
        const Rect& rect = geometricData.GetUnrotatedRect();
        SetAbsoluteRect(rect);
        for (uint32 i = 0; i < BORDERS_COUNT; ++i)
        {
            if (firstInit)
            {
                GetParent()->AddControl(borders[i]);
            }
            Rect borderRect = CreateFrameBorderRect(i, rect);
            borders[i]->SetAbsoluteRect(borderRect);
        }
        firstInit = false;
    }
    Rect CreateFrameBorderRect(uint32 border, const Rect& frameRect) const
    {
        switch (border)
        {
        case BORDER_TOP:
            return Rect(frameRect.x, frameRect.y, frameRect.dx, 1.0f);
        case BORDER_BOTTOM:
            return Rect(frameRect.x, frameRect.y + frameRect.dy, frameRect.dx, 1.0f);
        case BORDER_LEFT:
            return Rect(frameRect.x, frameRect.y, 1.0f, frameRect.dy);
        case BORDER_RIGHT:
            return Rect(frameRect.x + frameRect.dx, frameRect.y, 1.0f, frameRect.dy);
        default:
            DVASSERT("!impossible value for frame control position");
            return Rect();
        }
    }
    bool firstInit = true;
    Vector<ScopedPtr<UIControl>> borders;
};

class FrameRectControl : public ControlContainer
{
public:
    explicit FrameRectControl(const HUDAreaInfo::eArea area_)
        : ControlContainer(area_)
    {
        SetName("Frame Rect Control");
        background->SetSprite("~res:/Gfx/HUDControls/Rect", 0);
        background->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    }

private:
    void InitFromGD(const UIGeometricData& geometricData) override
    {
        Rect rect(Vector2(), FRAME_RECT_SIZE);
        rect.SetCenter(GetPos(geometricData));
        SetAbsoluteRect(rect);
    }

    Vector2 GetPos(const UIGeometricData& geometricData) const
    {
        Rect rect = geometricData.GetUnrotatedRect();
        Vector2 retVal = rect.GetPosition();
        switch (area)
        {
        case HUDAreaInfo::TOP_LEFT_AREA:
            return retVal;
        case HUDAreaInfo::TOP_CENTER_AREA:
            return retVal + Vector2(rect.dx / 2.0f, 0.0f);
        case HUDAreaInfo::TOP_RIGHT_AREA:
            return retVal + Vector2(rect.dx, 0.0f);
        case HUDAreaInfo::CENTER_LEFT_AREA:
            return retVal + Vector2(0, rect.dy / 2.0f);
        case HUDAreaInfo::CENTER_RIGHT_AREA:
            return retVal + Vector2(rect.dx, rect.dy / 2.0f);
        case HUDAreaInfo::BOTTOM_LEFT_AREA:
            return retVal + Vector2(0, rect.dy);
        case HUDAreaInfo::BOTTOM_CENTER_AREA:
            return retVal + Vector2(rect.dx / 2.0f, rect.dy);
        case HUDAreaInfo::BOTTOM_RIGHT_AREA:
            return retVal + Vector2(rect.dx, rect.dy);
        default:
            DVASSERT(!"wrong area passed to hud control");
            return Vector2(0.0f, 0.0f);
        }
    }
};

class PivotPointControl : public ControlContainer
{
public:
    explicit PivotPointControl()
        : ControlContainer(HUDAreaInfo::PIVOT_POINT_AREA)
    {
        SetName("pivot point control");
        background->SetSprite("~res:/Gfx/HUDControls/Pivot", 0);
        background->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    }

private:
    void InitFromGD(const UIGeometricData& geometricData) override
    {
        Rect rect(Vector2(), PIVOT_CONTROL_SIZE);
        const Rect& controlRect = geometricData.GetUnrotatedRect();
        rect.SetCenter(controlRect.GetPosition() + geometricData.pivotPoint * geometricData.scale);
        SetAbsoluteRect(rect);
    }
};

class RotateControl : public ControlContainer
{
public:
    explicit RotateControl()
        : ControlContainer(HUDAreaInfo::ROTATE_AREA)
    {
        SetName("rotate control");
        background->SetSprite("~res:/Gfx/HUDControls/Rotate", 2);
        background->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    }

private:
    void InitFromGD(const UIGeometricData& geometricData) override
    {
        Rect rect(Vector2(), ROTATE_CONTROL_SIZE);
        Rect controlRect = geometricData.GetUnrotatedRect();
        rect.SetCenter(Vector2(controlRect.GetPosition().x + controlRect.dx / 2.0f, controlRect.GetPosition().y - 20));
        SetAbsoluteRect(rect);
    }
};

ControlContainer* CreateControlContainer(HUDAreaInfo::eArea area)
{
    switch (area)
    {
    case HUDAreaInfo::PIVOT_POINT_AREA:
        return new PivotPointControl();
    case HUDAreaInfo::ROTATE_AREA:
        return new RotateControl();
    case HUDAreaInfo::TOP_LEFT_AREA:
    case HUDAreaInfo::TOP_CENTER_AREA:
    case HUDAreaInfo::TOP_RIGHT_AREA:
    case HUDAreaInfo::CENTER_LEFT_AREA:
    case HUDAreaInfo::CENTER_RIGHT_AREA:
    case HUDAreaInfo::BOTTOM_LEFT_AREA:
    case HUDAreaInfo::BOTTOM_CENTER_AREA:
    case HUDAreaInfo::BOTTOM_RIGHT_AREA:
        return new FrameRectControl(area);
    case HUDAreaInfo::FRAME_AREA:
        return new FrameControl();
    default:
        DVASSERT("!unacceptable value of area");
        return nullptr;
    }
}

HUDSystem::HUD::HUD(ControlNode* node_, UIControl* hudControl)
    : node(node_)
    , control(node_->GetControl())
    , container(new HUDContainer(control))
{
    HUDContainer* hudContainer = static_cast<HUDContainer*>(container.get());
    for (uint32 i = HUDAreaInfo::AREAS_BEGIN; i < HUDAreaInfo::AREAS_COUNT; ++i)
    {
        HUDAreaInfo::eArea area = static_cast<HUDAreaInfo::eArea>(i);
        ControlContainer* controlContainer = CreateControlContainer(area);
        hudContainer->AddChild(controlContainer);
        hudControls.emplace(std::piecewise_construct,
                            std::forward_as_tuple(area),
                            std::forward_as_tuple(controlContainer));
    }

    hudControl->AddControl(container);
}

HUDSystem::HUD::~HUD()
{
    for (auto control : hudControls)
    {
        container->RemoveControl(control.second);
    }
    container->RemoveFromParent();
}

void HUDSystem::HUD::UpdateHUDVisibility()
{
    const UIGeometricData& gd = control->GetGeometricData();
    bool visible = control->GetVisible() && control->GetVisibleForUIEditor() && gd.size.dx > 0.0f && gd.size.dy > 0.0f && gd.scale.dx > 0.0f && gd.scale.dy > 0.0f;
    container->SetVisible(visible);
}

HUDSystem::HUDSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
    , hudControl(new UIControl())
    , selectionRectControl(new SelectionRect())
    , sortedControlList(CompareByLCA)
{
    systemManager->GetPackage()->AddListener(this);
    hudControl->AddControl(selectionRectControl);
    hudControl->SetName("hudControl");
    systemManager->SelectionChanged.Connect(this, &HUDSystem::OnSelectionChanged);
    systemManager->EmulationModeChangedSignal.Connect(this, &HUDSystem::OnEmulationModeChanged);
    systemManager->EditingRootControlsChanged.Connect(this, &HUDSystem::OnRootContolsChanged);
}

HUDSystem::~HUDSystem()
{
    PackageNode* package = systemManager->GetPackage();
    if (nullptr != package)
    {
        systemManager->GetPackage()->RemoveListener(this);
    }
}

void HUDSystem::OnDeactivated()
{
    hudControl->RemoveFromParent();
    canDrawRect = false;
    selectionRectControl->SetSize(Vector2());
}

void HUDSystem::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    selectionContainer.MergeSelection(selected, deselected);
    if (!editingEnabled)
    {
        return;
    }
    for (auto node : deselected)
    {
        ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
        if (nullptr != controlNode)
        {
            hudMap.erase(controlNode);
            sortedControlList.erase(controlNode);
        }
    }

    for (auto node : selected)
    {
        ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
        if (controlNode != nullptr)
        {
            PackageBaseNode* parent = controlNode->GetParent();

            if (nullptr != controlNode && nullptr != controlNode->GetControl() && nullptr != parent && nullptr != parent->GetControl())
            {
                hudMap.emplace(std::piecewise_construct,
                               std::forward_as_tuple(controlNode),
                               std::forward_as_tuple(controlNode, hudControl));
                sortedControlList.insert(controlNode);
            }
        }
    }

    UpdateAreasVisibility();

    ProcessCursor(pressedPoint, SEARCH_BACKWARD);
}

bool HUDSystem::OnInput(UIEvent* currentInput)
{
    if (!editingEnabled)
    {
        return false;
    }
    switch (currentInput->phase)
    {
    case UIEvent::PHASE_MOVE:
        ProcessCursor(currentInput->point);
        return false;
    case UIEvent::PHASE_BEGAN:
    {
        //check that we can draw rect
        Vector<ControlNode*> nodes;
        systemManager->CollectControlNodesByPos(nodes, currentInput->point);
        const PackageControlsNode* packageNode = systemManager->GetPackage()->GetPackageControlsNode();
        bool noHudableControls = nodes.empty() || (nodes.size() == 1 && nodes.front()->GetParent() == packageNode);
        bool hotKeyDetected = InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_CTRL);
        SetCanDrawRect(hotKeyDetected || noHudableControls);
        pressedPoint = currentInput->point;
        return canDrawRect;
    }
    case UIEvent::PHASE_DRAG:
        dragRequested = true;

        if (canDrawRect)
        {
            Vector2 point(pressedPoint);
            Vector2 size(currentInput->point - pressedPoint);
            if (size.x < 0.0f)
            {
                point.x += size.x;
                size.x *= -1;
            }
            if (size.y <= 0.0f)
            {
                point.y += size.y;
                size.y *= -1;
            }
            selectionRectControl->SetAbsoluteRect(Rect(point, size));
            systemManager->SelectionRectChanged.Emit(selectionRectControl->GetAbsoluteRect());
        }
        return true;
    case UIEvent::PHASE_ENDED:
        ProcessCursor(currentInput->point);
        if (canDrawRect)
        {
            selectionRectControl->SetSize(Vector2());
        }
        bool retVal = dragRequested;
        SetCanDrawRect(false);
        dragRequested = false;
        return retVal;
    }
    return false;
}

void HUDSystem::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    const String& name = property->GetName();
    if (name == "Scale" || name == "Size" || name == "Visible")
    {
        for (auto& pair : hudMap)
        {
            pair.second.UpdateHUDVisibility();
        }
    }
}

void HUDSystem::OnRootContolsChanged(const EditorSystemsManager::SortedPackageBaseNodeSet& rootControls)
{
    SetEditingEnabled(rootControls.size() == 1);
}

void HUDSystem::OnEmulationModeChanged(bool emulationMode)
{
    if (emulationMode)
    {
        systemManager->GetRootControl()->RemoveControl(hudControl);
    }
    else
    {
        systemManager->GetRootControl()->AddControl(hudControl);
    }
}

void HUDSystem::ProcessCursor(const Vector2& pos, eSearchOrder searchOrder)
{
    SetNewArea(GetControlArea(pos, searchOrder));
}

HUDAreaInfo HUDSystem::GetControlArea(const Vector2& pos, eSearchOrder searchOrder) const
{
    uint32 end = HUDAreaInfo::AREAS_BEGIN;
    int sign = 1;
    if (searchOrder == SEARCH_BACKWARD)
    {
        if (HUDAreaInfo::AREAS_BEGIN != HUDAreaInfo::AREAS_COUNT)
        {
            end = HUDAreaInfo::AREAS_COUNT - 1;
        }
        sign = -1;
    }
    for (uint32 i = HUDAreaInfo::AREAS_BEGIN; i < HUDAreaInfo::AREAS_COUNT; ++i)
    {
        for (const auto& iter : sortedControlList)
        {
            ControlNode* node = dynamic_cast<ControlNode*>(iter);
            DVASSERT(nullptr != node);
            auto findIter = hudMap.find(node);
            DVASSERT_MSG(findIter != hudMap.end(), "hud map corrupted");
            const auto& hud = findIter->second;
            HUDAreaInfo::eArea area = static_cast<HUDAreaInfo::eArea>(end + sign * i);
            const auto& controlContainer = hud.hudControls.find(area)->second;
            if (controlContainer->GetVisible() && controlContainer->IsPointInside(pos))
            {
                return HUDAreaInfo(hud.node, area);
            }
        }
    }
    return HUDAreaInfo();
}

void HUDSystem::SetNewArea(const HUDAreaInfo& areaInfo)
{
    if (activeAreaInfo.area != areaInfo.area || activeAreaInfo.owner != areaInfo.owner)
    {
        activeAreaInfo = areaInfo;
        systemManager->ActiveAreaChanged.Emit(activeAreaInfo);
    }
}

void HUDSystem::SetCanDrawRect(bool canDrawRect_)
{
    if (canDrawRect != canDrawRect_)
    {
        canDrawRect = canDrawRect_;
        if (!canDrawRect)
        {
            selectionRectControl->SetSize(Vector2());
        }
    }
}

void HUDSystem::SetEditingEnabled(bool arg)
{
    if (editingEnabled != arg)
    {
        editingEnabled = arg;
        if (!editingEnabled)
        {
            hudMap.clear();
            sortedControlList.clear();
            selectionRectControl->SetSize(Vector2());
        }
        else
        {
            OnSelectionChanged(selectionContainer.selectedNodes, SelectedNodes());
        }
    }
}

void HUDSystem::UpdateAreasVisibility()
{
    bool showAreas = sortedControlList.size() == 1;
    for (const auto& iter : sortedControlList)
    {
        ControlNode* node = dynamic_cast<ControlNode*>(iter);
        DVASSERT(nullptr != node);
        auto findIter = hudMap.find(node);
        DVASSERT_MSG(findIter != hudMap.end(), "hud map corrupted");
        const auto& hud = findIter->second;
        for (HUDAreaInfo::eArea area : AreasToHide)
        {
            const auto& controlContainer = hud.hudControls.find(area)->second;
            controlContainer->SetVisible(showAreas);
        }
    }
}

