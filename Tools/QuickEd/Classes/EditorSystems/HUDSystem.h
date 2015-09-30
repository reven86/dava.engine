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

#ifndef __QUICKED_HUD_SYSTEM_H__
#define __QUICKED_HUD_SYSTEM_H__

#include "Base/ScopedPtr.h"
#include "Math/Vector.h"
#include "Math/Rect.h"
#include "UI/UIControl.h"
#include "EditorSystems/BaseEditorSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "Model/PackageHierarchy/PackageListener.h"
#include "Model/ControlProperties/RootProperty.h"

class ControlContainer : public DAVA::UIControl
{
public:
    explicit ControlContainer(const HUDAreaInfo::eArea area);

    HUDAreaInfo::eArea GetArea() const;
    virtual void InitFromGD(const DAVA::UIGeometricData& gd_) = 0;

protected:
    const HUDAreaInfo::eArea area = HUDAreaInfo::NO_AREA;
};

class HUDSystem final : public BaseEditorSystem, private PackageListener
{
public:
    HUDSystem(EditorSystemsManager* parent);
    ~HUDSystem() = default;

    void OnDeactivated() override;

    bool OnInput(DAVA::UIEvent* currentInput) override;

private:
    enum eSearchOrder
    {
        SEARCH_FORWARD,
        SEARCH_BACKWARD
    };

    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;

    void OnRootContolsChanged(const EditorSystemsManager::SortedPackageBaseNodeSet& rootControls);
    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void OnEmulationModeChanged(bool emulationMode);

    void ProcessCursor(const DAVA::Vector2& pos, eSearchOrder searchOrder = SEARCH_FORWARD);
    HUDAreaInfo GetControlArea(const DAVA::Vector2& pos, eSearchOrder searchOrder) const;
    void SetNewArea(const HUDAreaInfo& HUDAreaInfo);

    void SetCanDrawRect(bool canDrawRect_);
    void SetEditingEnabled(bool arg);
    void UpdateAreasVisibility();
    HUDAreaInfo activeAreaInfo;

    DAVA::ScopedPtr<DAVA::UIControl> hudControl;

    DAVA::Vector2 pressedPoint; //corner of selection rect
    bool canDrawRect = false; //selection rect state
    struct HUD
    {
        HUD(ControlNode* node, DAVA::UIControl* hudControl);
        ~HUD();
        void UpdateHUDVisibility();
        ControlNode* node = nullptr;
        DAVA::UIControl* control = nullptr;
        DAVA::ScopedPtr<ControlContainer> container;
        DAVA::Map<HUDAreaInfo::eArea, DAVA::ScopedPtr<ControlContainer>> hudControls;
    };
    DAVA::Map<ControlNode*, HUD> hudMap;
    DAVA::ScopedPtr<DAVA::UIControl> selectionRectControl;
    EditorSystemsManager::SortedPackageBaseNodeSet sortedControlList;
    bool dragRequested = false;
    bool editingEnabled = false;
    SelectionContainer selectionContainer;
};

#endif // __QUICKED_HUD_SYSTEM_H__
