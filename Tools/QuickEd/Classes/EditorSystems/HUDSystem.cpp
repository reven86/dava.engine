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

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

using namespace DAVA;

class HUDSystem::ControlContainer : public UIControl
{
public:
    explicit ControlContainer(const HUDAreaInfo::eArea area);

    HUDAreaInfo::eArea GetArea() const;
    virtual void InitFromGD(const UIGeometricData& gd_) = 0;
    void SetDPR(double arg);

protected:
    const HUDAreaInfo::eArea area = HUDAreaInfo::NO_AREA;
    double dpr = 1.0f;
};

HUDSystem::ControlContainer::ControlContainer(const HUDAreaInfo::eArea area_)
    : UIControl()
    , area(area_)
{
}

HUDAreaInfo::eArea HUDSystem::ControlContainer::GetArea() const
{
    return area;
}

void HUDSystem::ControlContainer::SetDPR(double arg)
{
    dpr = arg;
}

namespace
{
const Vector2 PIVOT_CONTROL_SIZE(10.0f, 10.0f);
const Vector2 FRAME_RECT_SIZE(7.5f, 7.5f);
const Vector2 ROTATE_CONTROL_SIZE(10.0f, 10.0f);
const float32 LINE_WIDTH = 0.5f;
const Array<HUDAreaInfo::eArea, 2> AreasToHide = {{HUDAreaInfo::PIVOT_POINT_AREA, HUDAreaInfo::ROTATE_AREA}};
}

class SelectionRect : public HUDSystem::ControlContainer
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
        : ControlContainer(HUDAreaInfo::NO_AREA)
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
    void InitFromGD(const UIGeometricData& geometricData) override
    {
        const Rect& rect = geometricData.GetUnrotatedRect();
        SetAbsoluteRect(rect);
        for (uint32 i = 0; i < BORDERS_COUNT; ++i)
        {
            Rect borderRect = CreateFrameBorderRect(i, rect);
            borders[i]->SetAbsoluteRect(borderRect);
        }
    }

private:
    void Draw(const UIGeometricData& geometricData) override
    {
        if (firstInit)
        {
            for (uint32 i = 0; i < BORDERS_COUNT; ++i)
            {
                AddControl(borders[i].get());
            }
            firstInit = false;
        }
        InitFromGD(geometricData);
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
            return Rect(frameRect.x, frameRect.y, frameRect.dx, LINE_WIDTH * dpr);
        case BORDER_BOTTOM:
            return Rect(frameRect.x, frameRect.y + frameRect.dy, frameRect.dx, LINE_WIDTH * dpr);
        case BORDER_LEFT:
            return Rect(frameRect.x, frameRect.y, LINE_WIDTH * dpr, frameRect.dy);
        case BORDER_RIGHT:
            return Rect(frameRect.x + frameRect.dx, frameRect.y, LINE_WIDTH * dpr, frameRect.dy);
        default:
            DVASSERT("!impossible value for frame control position");
            return Rect();
        }
    }
    bool firstInit = true;
    Vector<RefPtr<UIControl>> borders;
};

class HUDContainer : public HUDSystem::ControlContainer
{
public:
    explicit HUDContainer(UIControl* container);
    void AddChild(ControlContainer* container);
    void InitFromGD(const UIGeometricData& geometricData) override;
    void SystemDraw(const UIGeometricData& geometricData) override;

private:
    UIControl* control = nullptr;
    Vector<RefPtr<ControlContainer>> childs;
};

HUDContainer::HUDContainer(UIControl* container)
    : ControlContainer(HUDAreaInfo::NO_AREA)
    , control(container)
{
    SetName("HudContainer of " + container->GetName());
}

void HUDContainer::AddChild(ControlContainer* container)
{
    AddControl(container);
    childs.emplace_back(container);
}

void HUDContainer::InitFromGD(const UIGeometricData& geometricData)
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

void HUDContainer::SystemDraw(const UIGeometricData& geometricData)
{
    InitFromGD(control->GetGeometricData());
    UIControl::SystemDraw(geometricData);
}

class FrameControl : public HUDSystem::ControlContainer
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
                GetParent()->AddControl(borders[i].Get());
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
            return Rect(frameRect.x, frameRect.y, frameRect.dx, LINE_WIDTH * dpr);
        case BORDER_BOTTOM:
            return Rect(frameRect.x, frameRect.y + frameRect.dy, frameRect.dx, LINE_WIDTH * dpr);
        case BORDER_LEFT:
            return Rect(frameRect.x, frameRect.y, LINE_WIDTH * dpr, frameRect.dy);
        case BORDER_RIGHT:
            return Rect(frameRect.x + frameRect.dx, frameRect.y, LINE_WIDTH * dpr, frameRect.dy);
        default:
            DVASSERT("!impossible value for frame control position");
            return Rect();
        }
    }
    bool firstInit = true;
    Vector<RefPtr<UIControl>> borders;
};

class FrameRectControl : public HUDSystem::ControlContainer
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
        Rect rect(Vector2(), FRAME_RECT_SIZE * dpr);
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

class PivotPointControl : public HUDSystem::ControlContainer
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
        Rect rect(Vector2(), PIVOT_CONTROL_SIZE * dpr);
        const Rect& controlRect = geometricData.GetUnrotatedRect();
        rect.SetCenter(controlRect.GetPosition() + geometricData.pivotPoint * geometricData.scale);
        SetAbsoluteRect(rect);
    }
};

class RotateControl : public HUDSystem::ControlContainer
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
        Rect rect(Vector2(), ROTATE_CONTROL_SIZE * dpr);
        Rect controlRect = geometricData.GetUnrotatedRect();
        rect.SetCenter(Vector2(controlRect.GetPosition().x + controlRect.dx / 2.0f, controlRect.GetPosition().y - 20));
        SetAbsoluteRect(rect);
    }
};

HUDSystem::ControlContainer* CreateControlContainer(HUDAreaInfo::eArea area)
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

struct HUDSystem::HUD
{
    HUD(ControlNode* node, UIControl* hudControl);
    ~HUD();
    void UpdateHUDVisibility();
    ControlNode* node = nullptr;
    UIControl* control = nullptr;
    UIControl* hudControl = nullptr;
    RefPtr<HUDContainer> container;
    Map<HUDAreaInfo::eArea, RefPtr<UIControl>> hudControls;
};

HUDSystem::HUD::HUD(ControlNode* node_, UIControl* hudControl_)
    : node(node_)
    , control(node_->GetControl())
    , hudControl(hudControl_)
    , container(new HUDContainer(control))
{
    container->SetName("Container for HUD controls of node " + node_->GetName());
    uint32 begin = HUDAreaInfo::AREAS_BEGIN;
    uint32 end = HUDAreaInfo::AREAS_COUNT;
    if (node->GetParent() == nullptr || node->GetParent()->GetControl() == nullptr)
    {
        begin = HUDAreaInfo::CORNERS_BEGIN;
        end = HUDAreaInfo::CORNERS_COUNT;
    }
    for (uint32 i = begin; i < end; ++i)
    {
        HUDAreaInfo::eArea area = static_cast<HUDAreaInfo::eArea>(i);
        ControlContainer* controlContainer = CreateControlContainer(area);
        container->AddChild(controlContainer);
        hudControls[area] = controlContainer;
    }
    hudControl->AddControl(container.Get());
    container->InitFromGD(control->GetGeometricData());
}

HUDSystem::HUD::~HUD()
{
    hudControl->RemoveControl(container.Get());
}

void HUDSystem::HUD::UpdateHUDVisibility()
{
    const UIGeometricData& gd = control->GetGeometricData();
    bool visible = control->GetSystemVisible() && gd.size.dx > 0.0f && gd.size.dy > 0.0f && gd.scale.dx > 0.0f && gd.scale.dy > 0.0f;
    container->SetVisible(visible);
}

HUDSystem::HUDSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
    , hudControl(new UIControl())
    , selectionRectControl(new SelectionRect())
    , sortedControlList(CompareByLCA)
{
    systemManager->GetPackage()->AddListener(this);
    hudControl->AddControl(selectionRectControl.Get());
    hudControl->SetName("hudControl");
    systemManager->SelectionChanged.Connect(this, &HUDSystem::OnSelectionChanged);
    systemManager->EmulationModeChangedSignal.Connect(this, &HUDSystem::OnEmulationModeChanged);
    systemManager->DPRChanged.Connect(this, &HUDSystem::OnDPRChanged);
    systemManager->EditingRootControlsChanged.Connect(this, &HUDSystem::OnRootContolsChanged);
    systemManager->MagnetLinesChanged.Connect(this, &HUDSystem::OnMagnetLinesChanged);
}

HUDSystem::~HUDSystem()
{
    PackageNode* package = systemManager->GetPackage();
    if (nullptr != package)
    {
        package->RemoveListener(this);
    }
    systemManager->GetRootControl()->RemoveControl(hudControl.Get());
}

void HUDSystem::OnActivated()
{
    systemManager->GetRootControl()->AddControl(hudControl.Get());
}

void HUDSystem::OnDeactivated()
{
    systemManager->GetRootControl()->RemoveControl(hudControl.Get());
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
            if (nullptr != controlNode && nullptr != controlNode->GetControl())
            {
                hudMap.emplace(std::piecewise_construct,
                               std::forward_as_tuple(controlNode),
                               std::forward_as_tuple(new HUD(controlNode, hudControl.Get())));
                for (auto& hudControlsIter : hudMap.at(controlNode)->hudControls)
                {
                    hudControlsIter.second->SetDPR(dpr);
                }
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
                size.x *= -1.0f;
            }
            if (size.y <= 0.0f)
            {
                point.y += size.y;
                size.y *= -1.0f;
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
        for (auto& hudPair : hudMap)
        {
            hudPair.second->UpdateHUDVisibility();
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
        systemManager->GetRootControl()->RemoveControl(hudControl.Get());
    }
    else
    {
        systemManager->GetRootControl()->AddControl(hudControl.Get());
    }
}

void HUDSystem::OnMagnetLinesChanged(const Vector<MagnetLineInfo>& magnetLines)
{
    static const float32 axtraSizeValue = 50.0f;
    for (auto& magnetControl : magnetControls)
    {
        hudControl->RemoveControl(magnetControl.Get());
    }
    magnetControls.clear();

    for (const MagnetLineInfo& line : magnetLines)
    {
        UIControl* control = new UIControl();
        control->SetDebugDraw(true);
        Rect lineRect = line.absoluteRect;
        UIControl* topLevelControl = hudControl.Get();
        while (topLevelControl->GetParent()->GetParent() != nullptr) //first control is screen
        {
            topLevelControl = topLevelControl->GetParent();
        }
        lineRect.SetPosition(line.absoluteRect.GetPosition() - topLevelControl->GetPosition());
        control->SetAbsoluteRect(lineRect);
        Vector2 extraSize(line.axis == Vector2::AXIS_X ? axtraSizeValue : 0.0f, line.axis == Vector2::AXIS_Y ? axtraSizeValue : 0.0f);
        control->SetSize(control->GetSize() + extraSize);
        control->SetAngle(line.gd->angle);
        control->SetPivotPoint(extraSize / 2.0f);
        hudControl->AddControl(control);
        magnetControls.emplace_back(control);
    }
}

void HUDSystem::OnDPRChanged(double arg)
{
    dpr = arg;
    selectionRectControl->SetDPR(dpr);
    for (auto& hudMapIter : hudMap)
    {
        for (auto& hudControlsIter : hudMapIter.second->hudControls)
        {
            hudControlsIter.second->SetDPR(dpr);
        }
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
            auto hudControlsIter = hud->hudControls.find(area);
            if (hudControlsIter != hud->hudControls.end())
            {
                const auto& controlContainer = hudControlsIter->second;
                if (controlContainer->GetSystemVisible() && controlContainer->IsPointInside(pos))
                {
                    return HUDAreaInfo(hud->node, area);
                }
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
            auto hudControlsIter = hud->hudControls.find(area);
            if (hudControlsIter != hud->hudControls.end())
            {
                const auto& controlContainer = hudControlsIter->second;
                controlContainer->SetVisible(showAreas);
            }
        }
    }
}

