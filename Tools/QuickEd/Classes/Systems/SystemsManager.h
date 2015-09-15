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

#ifndef __QUICKED_SYSTEMS_MANAGER_H__
#define __QUICKED_SYSTEMS_MANAGER_H__

#include "Base/BaseTypes.h"
#include "Functional/Signal.h"
#include "Systems/SelectionContainer.h"
#include "Math/Rect.h"
#include "Math/Vector.h"

struct HUDAreaInfo
{
    enum eArea
    {
        TOP_LEFT_AREA,
        TOP_CENTER_AREA,
        TOP_RIGHT_AREA,
        CENTER_LEFT_AREA,
        CENTER_RIGHT_AREA,
        BOTTOM_LEFT_AREA,
        BOTTOM_CENTER_AREA,
        BOTTOM_RIGHT_AREA,
        FRAME_AREA,
        PIVOT_POINT_AREA,
        ROTATE_AREA,
        NO_AREA,
        CORNERS_COUNT = FRAME_AREA - TOP_LEFT_AREA,
        AREAS_COUNT = NO_AREA - TOP_LEFT_AREA
    };
    ControlNode* owner = nullptr;
    eArea area = NO_AREA;
};

namespace DAVA
{
class UIControl;
class UIEvent;
class VariantType;
}

class BaseSystem;
class AbstractProperty;
class PackageNode;

class SystemsManager
{
public:
    explicit SystemsManager(PackageNode* package);
    ~SystemsManager();

    PackageNode* GetPackage();

    DAVA::UIControl* GetRootControl();
    DAVA::UIControl* GetScalableControl();

    void Deactivate();
    void Activate();

    bool OnInput(DAVA::UIEvent* currentInput);

    bool IsInEmulationMode() const;
    void SetEmulationMode(bool emulationMode);

    void GetControlNodesByPos(DAVA::Vector<ControlNode*>& controlNodes, const DAVA::Vector2& pos) const;
    void GetControlNodesByRect(SelectedControls& controlNodes, const DAVA::Rect& rect) const;

    DAVA::Signal<const SelectedNodes& /*selected*/, const SelectedNodes& /*deselected*/> SelectionChanged;
    DAVA::Signal<const HUDAreaInfo& /*areaInfo*/> ActiveAreaChanged;
    DAVA::Signal<const DAVA::Rect& /*selectionRectControl*/> SelectionRectChanged;
    DAVA::Signal<bool> EmulationModeChangedSignal;
    DAVA::Signal<> CanvasSizeChanged;
    DAVA::Signal<ControlNode* /*node*/, AbstractProperty* /*prop*/, const DAVA::VariantType& /*value*/> PropertyChanged;
    DAVA::Signal<const DAVA::Vector<ControlNode*>& /*nodes*/, const DAVA::Vector2& /*pos*/, ControlNode*& /*selectedNode*/> SelectionByMenuRequested;

private:
    void GetControlNodesByPosImpl(DAVA::Vector<ControlNode*>& controlNodes, const DAVA::Vector2& pos, ControlNode* node) const;
    void GetControlNodesByRectImpl(SelectedControls& controlNodes, const DAVA::Rect& rect, ControlNode* node) const;

    DAVA::UIControl* rootControl = nullptr;
    DAVA::UIControl* scalableControl = nullptr;

    DAVA::List<BaseSystem*> systems;

    PackageNode* package = nullptr;
    static const bool emulationByDefault = false;
    bool emulationMode = emulationByDefault;
};

#endif // __QUICKED_SYSTEMS_MANAGER_H__
