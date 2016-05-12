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


#ifndef __QUICKED_TRANSFORM_SYSTEM_H__
#define __QUICKED_TRANSFORM_SYSTEM_H__


#include "Base/BaseTypes.h"
#include "Math/Vector.h"
#include "UI/UIControl.h"
#include "Preferences/PreferencesRegistrator.h"
#include "Input/KeyboardDevice.h"
#include "EditorSystems/BaseEditorSystem.h"
#include "EditorSystems/EditorSystemsManager.h"

namespace DAVA
{
class UIGeometricData;
class UIControl;
}

class EditorTransformSystem : public DAVA::InspBase, public BaseEditorSystem
{
public:
    explicit EditorTransformSystem(EditorSystemsManager* parent);

private:
    enum eDirections
    {
        NEGATIVE_DIRECTION = -1,
        NO_DIRECTION = 0,
        POSITIVE_DIRECTION = 1
    };
    using Directions = DAVA::Array<int, DAVA::Vector2::AXIS_COUNT>;
    using CornersDirections = DAVA::Array<Directions, HUDAreaInfo::CORNERS_COUNT>;
    static const CornersDirections cornersDirections;

    struct MoveInfo;

    bool OnInput(DAVA::UIEvent* currentInput) override;

    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void OnActiveAreaChanged(const HUDAreaInfo& areaInfo);

    bool ProcessKey(DAVA::Key key);
    bool ProcessDrag(DAVA::Vector2 point);

    void ResizeControl(DAVA::Vector2 delta, bool withPivot, bool rateably);
    DAVA::Vector2 AdjustResizeToMinimumSize(DAVA::Vector2 delta);
    DAVA::Vector2 AdjustResizeToBorderAndToMinimum(DAVA::Vector2 deltaSize, DAVA::Vector2 transformPoint, Directions directions);
    DAVA::Vector2 AdjustResizeToBorder(DAVA::Vector2 deltaSize, DAVA::Vector2 transformPoint, Directions directions, DAVA::Vector<MagnetLineInfo>& magnets);

    void MovePivot(DAVA::Vector2 delta);
    DAVA::Vector2 AdjustPivotToNearestArea(DAVA::Vector2& delta);

    bool Rotate(DAVA::Vector2 pos);
    DAVA::float32 AdjustRotateToFixedAngle(DAVA::float32 deltaAngle, DAVA::float32 originalAngle);

    void MoveAllSelectedControls(DAVA::Vector2 delta, bool canAdjust);
    DAVA::Vector2 AdjustMoveToNearestBorder(DAVA::Vector2 delta, DAVA::Vector<MagnetLineInfo>& magnetLines, const DAVA::UIGeometricData* parentGD, const DAVA::UIControl* control);

    void CorrectNodesToMove();
    void UpdateNeighboursToMove();

    void ClampAngle();
    struct MagnetLine;
    DAVA::Vector<MagnetLine> CreateMagnetPairs(const DAVA::Rect& box, const DAVA::UIGeometricData* parentGD, const DAVA::Vector<DAVA::UIControl*>& neighbours, DAVA::Vector2::eAxis axis);
    void ExtractMatchedLines(DAVA::Vector<MagnetLineInfo>& magnets, const DAVA::Vector<MagnetLine>& magnetLines, const DAVA::UIControl* control, DAVA::Vector2::eAxis axis);
    bool IsShiftPressed() const;
    DAVA::size_type currentHash = 0;
    HUDAreaInfo::eArea activeArea = HUDAreaInfo::NO_AREA;
    ControlNode* activeControlNode = nullptr;
    DAVA::Vector2 prevPos;
    DAVA::Vector2 extraDelta;
    SelectedControls selectedControlNodes;
    DAVA::List<std::unique_ptr<MoveInfo>> nodesToMoveInfos;
    DAVA::Vector<DAVA::UIControl*> neighbours;

    DAVA::UIGeometricData parentGeometricData;
    DAVA::UIGeometricData controlGeometricData;
    AbstractProperty* sizeProperty = nullptr;
    AbstractProperty* positionProperty = nullptr;
    AbstractProperty* angleProperty = nullptr;
    AbstractProperty* pivotProperty = nullptr;

    DAVA::Vector2 magnetRange;
    DAVA::float32 moveStepByKeyboard;
    DAVA::float32 expandedMoveStepByKeyboard;
    DAVA::Vector2 borderInParentToMagnet;
    DAVA::Vector2 indentOfControlToManget;
    DAVA::Vector2 shareOfSizeToMagnetPivot;
    DAVA::float32 angleSegment;
    bool shiftInverted;

public:
    INTROSPECTION(EditorTransformSystem,
                  MEMBER(magnetRange, "Control Transformations/Mouse magnet distance", DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  MEMBER(moveStepByKeyboard, "Control Transformations/Move distance by keyboard", DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  MEMBER(expandedMoveStepByKeyboard, "Control Transformations/Move distance by keyboard alternate", DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  MEMBER(borderInParentToMagnet, "Control Transformations/Magnet distance inside", DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  MEMBER(indentOfControlToManget, "Control Transformations/Magnet distance outside", DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  MEMBER(shareOfSizeToMagnetPivot, "Control Transformations/Pivot magnet share", DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  MEMBER(angleSegment, "Control Transformations/Rotate section angle", DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  MEMBER(shiftInverted, "Control Transformations/Invert shift button", DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  )

    REGISTER_PREFERENCES(EditorTransformSystem)
};

#endif // __QUICKED_TRANSFORM_SYSTEM_H__
