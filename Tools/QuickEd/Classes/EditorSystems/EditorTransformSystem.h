
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

    DAVA::Vector2 moveMagnetRange;
    DAVA::Vector2 resizeMagnetRange;
    DAVA::Vector2 pivotMagnetRange;
    DAVA::float32 moveStepByKeyboard;
    DAVA::float32 expandedMoveStepByKeyboard;
    DAVA::Vector2 borderInParentToMagnet;
    DAVA::Vector2 indentOfControlToManget;
    DAVA::Vector2 shareOfSizeToMagnetPivot;
    DAVA::float32 angleSegment;
    bool shiftInverted;

public:
    INTROSPECTION(EditorTransformSystem,
                  MEMBER(moveMagnetRange, "Control Transformations/Mouse magnet distance on move", DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  MEMBER(resizeMagnetRange, "Control Transformations/Mouse magnet distance on resize", DAVA::I_SAVE | DAVA::I_PREFERENCE)
                  MEMBER(pivotMagnetRange, "Control Transformations/Mouse magnet distance on move pivot point", DAVA::I_SAVE | DAVA::I_PREFERENCE)
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
