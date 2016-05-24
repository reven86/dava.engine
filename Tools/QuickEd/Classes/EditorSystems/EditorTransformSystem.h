#ifndef __QUICKED_TRANSFORM_SYSTEM_H__
#define __QUICKED_TRANSFORM_SYSTEM_H__

#include "EditorSystems/BaseEditorSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "Base/BaseTypes.h"
#include "Math/Vector.h"
#include "UI/UIControl.h"
#include <array>

namespace DAVA
{
class UIGeometricData;
class UIControl;
}

class EditorTransformSystem final : public BaseEditorSystem
{
public:
    explicit EditorTransformSystem(EditorSystemsManager* parent);
    ~EditorTransformSystem();

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

    bool ProcessKey(const DAVA::Key key);
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

    size_t currentHash = 0;
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
};

#endif // __QUICKED_TRANSFORM_SYSTEM_H__
