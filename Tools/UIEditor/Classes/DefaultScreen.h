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
	void SetPos(const Vector2& pos);

	Vector2 LocalToInternal(const Vector2& localPoint) const;
	
	virtual void Input(UIEvent * touch);
	virtual bool SystemInput(UIEvent *currentInput);
	
	Qt::CursorShape GetCursor(const Vector2&);
	
	void BacklightControl(const Vector2& pos);
	bool IsDropEnable(const Vector2& pos)const;
	
private:
	enum InputState
	{
		InputStateSelection,
		InputStateDrag,
		InputStateSize,
		InputStateSelectorControl,
		InputStateScreenMove
	};
	
	enum ResizeType
	{
		ResizeTypeNoResize,
		ResizeTypeLeft,
		ResizeTypeRight,
		ResizeTypeTop,
		ResizeTypeBottom,
		ResizeTypeLeftTop,
		ResizeTypeLeftBottom,
		ResizeTypeRigthTop,
		ResizeTypeRightBottom,
		ResizeTypeMove,
	};
	
	HierarchyTreeControlNode* GetSelectedControl(const Vector2& point, const HierarchyTreeNode* parent) const;
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
	HierarchyTreeControlNode* SmartGetSelectedControl(const Vector2& point);
	void SmartGetSelectedControl(SmartSelection* list, const HierarchyTreeNode* parent, const Vector2& point);
	HierarchyTreeControlNode* GetSelectedControl(const Vector2& point);
	
	void ApplyMoveDelta(const Vector2& delta);
	HierarchyTreeController::SELECTEDCONTROLNODES GetActiveMoveControls() const;
	void ResetMoveDelta();
	void SaveControlsPostion();
	void MoveControl(const Vector2& delta);

	void DeleteSelectedControls();
	
	void ApplySizeDelta(const Vector2& delta);
	bool IsNeedApplyResize() const;
	void ResetSizeDelta();
	void ResizeControl();
	ResizeType GetResizeType(const HierarchyTreeControlNode* selectedControlNode, const Vector2& point) const;
	Qt::CursorShape ResizeTypeToQt(ResizeType);

	void ApplyMouseSelection(const Vector2& rectSize);
	
	void MouseInputBegin(const DAVA::UIEvent* event);
	void MouseInputEnd(const DAVA::UIEvent* event);
	void MouseInputDrag(const DAVA::UIEvent* event);
	void KeyboardInput(const DAVA::UIEvent* event);
	
	Vector2 GetInputDelta(const Vector2& point, bool applyScale = true) const;
	
	Rect GetControlRect(const HierarchyTreeControlNode* control) const;
	void CopySelectedControls();
	
private:
	Vector2 scale;
	Vector2 pos;
	
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
	
	// Verify if the point is iside control's rect. Extend control's rect with delta.
	bool IsPointInsideRectWithDelta(const Rect& rect, const Vector2& point, int32 pointDelta = 0) const;

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

	// Check control's visibility in a recursive way.
	bool IsControlVisible(UIControl* uiControl);
	void IsControlVisibleRecursive(const UIControl* uiControl, bool& isVisible);

private slots:
	void ControlContextMenuTriggered(QAction* action);
};