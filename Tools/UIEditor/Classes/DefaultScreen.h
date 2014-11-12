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


#include "DAVAEngine.h"
#include <QObject>
#include <qnamespace.h>
#include "HierarchyTreeNode.h"
#include "HierarchyTreeController.h"

#include "Helpers/UIControlResizeHelper.h"

using namespace DAVA;

class QAction;
class HierarchyTreeControlNode;

class DefaultScreen : public QObject, public UIScreen
{
	Q_OBJECT
	
	const static int32 MIN_CONTROL_SIZE = 1;
	
protected:
	virtual ~DefaultScreen();
public:
	DefaultScreen();
	
	virtual void Draw(const UIGeometricData &geometricData);
	virtual void SystemDraw(const UIGeometricData &geometricData);
	virtual void Update(float32 timeElapsed);
	virtual bool IsPointInside(const Vector2& point, bool expandWithFocus);

	void SetScale(const Vector2& scale);
    Vector2 GetScale() const {return scale;};

	void SetPos(const Vector2& pos);
    const Vector2& GetPos() const {return pos;};

	Vector2 LocalToInternal(const Vector2& localPoint) const;
	
	virtual void Input(UIEvent * touch);
	virtual bool SystemInput(UIEvent *currentInput);
	
	Qt::CursorShape GetCursor(const Vector2&);
	
	void BacklightControl(const Vector2& pos);
	bool IsDropEnable(const Vector2& pos)const;
	
    void SetScreenControl(ScreenControl* control);
    ScreenControl* GetScreenControl() const;

    // Screen scale/position is changed.
    void SetScreenScaleChangedFlag();
    void SetScreenPositionChangedFlag();

signals:
    void DeleteNodes(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodesList);
    
private:
	enum InputState
	{
		InputStateSelection,
		InputStateDrag,
		InputStateSize,
		InputStateSelectorControl,
		InputStateScreenMove,
        InputStateGuideMove
	};
	
    enum eKeyboardMoveDirection
    {
        moveUp,
        moveDown,
        moveLeft,
        moveRight
    };

    struct InputDelta
    {
        InputDelta(const Vector2& moveDelta, bool stickedToX, bool stickedToY)
        {
            delta = moveDelta;
            isStickedToX = stickedToX;
            isStickedToY = stickedToY;
        }

        Vector2 delta;
        bool isStickedToX;
        bool isStickedToY;
    };
    
	void GetSelectedControl(HierarchyTreeNode::HIERARCHYTREENODESLIST& list, const Rect& rect, const HierarchyTreeNode* parent) const;
	
	class SmartSelection
	{
	public:
		typedef std::vector<HierarchyTreeNode::HIERARCHYTREENODEID> SelectionVector;
		SmartSelection(HierarchyTreeNode::HIERARCHYTREENODEID id);
		
		bool IsEqual(const SmartSelection* item) const;
		HierarchyTreeNode::HIERARCHYTREENODEID GetFirst() const;
		HierarchyTreeNode::HIERARCHYTREENODEID GetLast() const;
		HierarchyTreeNode::HIERARCHYTREENODEID GetNext(HierarchyTreeNode::HIERARCHYTREENODEID id) const;
		SelectionVector GetAll() const;
	private:
		void FormatSelectionVector(SelectionVector& selection) const;
	public:
		class childsSet: public std::vector<SmartSelection *>
		{
		public:
			virtual ~childsSet();
		};
		SmartSelection* parent;
		childsSet childs;
		HierarchyTreeNode::HIERARCHYTREENODEID id;
	};
	HierarchyTreeControlNode* SmartGetSelectedControl(const Vector2& point) const;
	void SmartGetSelectedControl(SmartSelection* list, const HierarchyTreeNode* parent, const Vector2& point) const;
	HierarchyTreeControlNode* GetSelectedControl(const Vector2& point);
	
	void ApplyMoveDelta(const Vector2& delta);
	HierarchyTreeController::SELECTEDCONTROLNODES GetActiveMoveControls() const;
	void ResetMoveDelta();
	void SaveControlsPostion();

    // Entry point for performing move from keyboard.
    void DoKeyboardMove(eKeyboardMoveDirection moveDirection);

	void MoveControl(const Vector2& delta, bool alignControlToIntegerPos);

    void MoveGuide(HierarchyTreeScreenNode* screenNode);
    void MoveGuides(eKeyboardMoveDirection moveDirection, const Vector2& delta);

	void DeleteSelectedControls();
    void DeleteSelectedGuides(HierarchyTreeScreenNode* screenNode);
	
	void ApplySizeDelta(const Vector2& delta);
	bool IsNeedApplyResize() const;
	void ResetSizeDelta();
	void ResizeControl();
	ResizeType GetResizeType(const HierarchyTreeControlNode* selectedControlNode, const Vector2& point) const;
    Qt::CursorShape ResizeTypeToQt(ResizeType resize, const HierarchyTreeControlNode* selectedNode);
	void ApplyMouseSelection(const Vector2& rectSize);
	
	void MouseInputBegin(const DAVA::UIEvent* event);
	void MouseInputEnd(const DAVA::UIEvent* event);
	void MouseInputDrag(const DAVA::UIEvent* event);
	void KeyboardInput(const DAVA::UIEvent* event);
	
	InputDelta GetInputDelta(const Vector2& point, bool applyScale = true);
	
	Rect GetControlRect(const HierarchyTreeControlNode* control, bool checkAngle = false) const;
	void CopySelectedControls();

    // In case Preview mode is enabled, translate mouse UI events directly to the preview screen.
    UIEvent* PreprocessEventForPreview(UIEvent* event);

private:
	Vector2 scale;
	Vector2 pos;
    Vector2 viewSize;

    bool isStickedToX;
    bool isStickedToY;
    Vector2 stickDelta;
    Vector2 prevDragPoint;

	InputState inputState;
	ResizeType resizeType;
	Rect resizeRect;
	Vector2 inputPos;
	typedef Map<const UIControl*, Vector2> MAP_START_CONTROL_POS;
	MAP_START_CONTROL_POS startControlPos;
	HierarchyTreeControlNode* lastSelectedControl;
	bool copyControlsInProcess;
	//This flag should prevent additional control selection in MouseInputEnd event handler
	bool useMouseUpSelection;
	
	UIControl* selectorControl;

    // Screen currently displayed in UIEditor (might be NULL).
    ScreenControl* screenControl;

    bool isNeedHandleScreenScaleChanged;
    bool isNeedHandleScreenPositionChanged;

    // Verify whether the point is inside control, taking its angle into account.
    bool IsPointInsideControlWithDelta(UIControl* uiControl, const Vector2& point, int32 pointDelta) const;

    // Calculate the distance between control's rect (including rotated one) and point.
    Vector4 CalculateDistancesToControlBounds(UIControl* uiControl, const Vector2& point) const;

	// Whether the Mouse Begin event happened?
	bool mouseAlreadyPressed;
	
	void HandleMouseRightButtonClick(const Vector2& point);
	void HandleMouseLeftButtonClick(const Vector2& point);	
	void ShowControlContextMenu(const SmartSelection& selection);

	// Screen Move functionality.
	bool CheckEnterScreenMoveState();
	void CheckScreenMoveState();
	void CheckExitScreenMoveState();

	void HandleScreenMove(const DAVA::UIEvent* event);

	// Get the state of the "Move Screen" key.
	bool IsMoveScreenKeyPressed();

	// Get the control/guide move delta (coarse/fine, depending on whether Shift key is pressed).
	int32 GetControlMoveDelta();
    int32 GetGuideMoveDelta();

	// Check control's visibility.
	bool IsControlVisible(const UIControl* uiControl) const;

    // Calculate the stick to guides for different input modes.
    int32 CalculateStickToGuides(Vector2& offset) const;
    int32 CalculateStickToGuidesDrag(Vector2& offset) const;
    int32 CalculateStickToGuidesSize(Vector2& offset) const;

    // Helper stick calculation functions.
    int32 CalculateStickToRectBounds(HierarchyTreeScreenNode* screenNode, const Rect& selectedRect, Vector2& offset) const;
    int32 CalculateStickToRectCenter(HierarchyTreeScreenNode* screenNode, const Rect& selectedRect, Vector2& offset) const;

    // Do we need to calculate stick mode at all?
    bool NeedCalculateStickMode(HierarchyTreeScreenNode* screenNode) const;

    // Get the stick treshold.
    int32 GetGuideStickTreshold() const;

    // Draw the guides.
    void DrawGuides();

    // Handle the "screen scale/screen position" change.
    void HandleScreenScalePositionChanged();

private slots:
	void ControlContextMenuTriggered(QAction* action);
};