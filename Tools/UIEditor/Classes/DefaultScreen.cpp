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


#include "DefaultScreen.h"
#include "ScreenWrapper.h"
#include "ControlCommands.h"
#include "ItemsCommand.h"
#include "CommandsController.h"
#include "CopyPasteController.h"
#include "HierarchyTreeAggregatorControlNode.h"

#include "Grid/GridController.h"
#include "Ruler/RulerController.h"

#include "PreviewController.h"

#include <QMenu>
#include <QAction>
#include <QApplication>

#define SIZE_CURSOR_DELTA 5
#define MIN_DRAG_DELTA 3

// Coarse/Fine Control Move delta.
#define COARSE_CONTROL_MOVE_DELTA 10
#define FINE_CONTROL_MOVE_DELTA 1

#define MOVE_SCREEN_KEY DVKEY_SPACE

const char* MENU_PROPERTY_ID = "id";

class UISelectorControl: public UIControl
{
public:
	UISelectorControl()
	{
		RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);		
	}
	
	void SystemDraw(const UIGeometricData &/*geometricData*/)
	{
		RenderHelper::Instance()->DrawRect(GetRect(), RenderState::RENDERSTATE_2D_BLEND);
	}
};

DefaultScreen::DefaultScreen()
{
	scale = Vector2(1.f, 1.f);
	pos = Vector2(0, 0);
	
	inputState = InputStateSelection;
	lastSelectedControl = NULL;
	
	selectorControl = new UISelectorControl();
    screenControl = NULL;
	
	copyControlsInProcess = false;
	mouseAlreadyPressed = false;
}

DefaultScreen::~DefaultScreen()
{
	SafeRelease(selectorControl);
}

void DefaultScreen::Update(float32 /*timeElapsed*/)
{
	CheckScreenMoveState();

	//update view port
	RenderManager::Instance()->SetDrawScale(scale);
	RenderManager::Instance()->SetDrawTranslate(pos);
}

void DefaultScreen::Draw(const UIGeometricData &geometricData)
{
	UIScreen::Draw(geometricData);
}

void DefaultScreen::SystemDraw(const UIGeometricData &geometricData)
{
    bool previewEnabled = PreviewController::Instance()->IsPreviewEnabled();
    Color oldColor = RenderManager::Instance()->GetColor();

    RenderManager::Instance()->SetColor(ScreenWrapper::Instance()->GetBackgroundFrameColor());
    RenderHelper::Instance()->FillRect(ScreenWrapper::Instance()->GetBackgroundFrameRect(), RenderState::RENDERSTATE_2D_BLEND);
    RenderManager::Instance()->SetColor(oldColor);

   // For Preview mode display only what is inside the preview rectangle.
    if (previewEnabled)
    {
        RenderManager::Instance()->ClipPush();
        
        Rect previewClipRect;
        previewClipRect.SetSize(PreviewController::Instance()->GetTransformData().screenSize);
        RenderManager::Instance()->ClipRect(previewClipRect);
    }

	UIScreen::SystemDraw(geometricData);

    if (previewEnabled)
    {
        RenderManager::Instance()->ClipPop();
    }
    else if (inputState == InputStateSelectorControl)
    {
		selectorControl->SystemDraw(geometricData);
    }
}

bool DefaultScreen::IsPointInside(const Vector2& /*point*/, bool /*expandWithFocus*/)
{
	//YZ:
	//all input must be pased to input handler
	return true;
}


Vector2 DefaultScreen::LocalToInternal(const Vector2& localPoint) const
{
	Vector2 point = -pos;
	point.x += localPoint.x / scale.x;
	point.y += localPoint.y / scale.y;
	return point;
}

void DefaultScreen::SetScale(const Vector2& scale)
{
	this->scale = scale;
    if (screenControl)
    {
        screenControl->SetScale(scale);
    }
}

void DefaultScreen::SetPos(const Vector2& pos)
{
	this->pos = pos;
    if (screenControl)
    {
        screenControl->SetPos(pos);
    }
}

void DefaultScreen::Input(DAVA::UIEvent* event)
{
    if (PreviewController::Instance()->IsPreviewEnabled())
    {
        UIScreen::Input(PreprocessEventForPreview(event));
        return;
    }

	switch (event->phase)
	{
		case UIEvent::PHASE_BEGAN:
		{
			MouseInputBegin(event);
		}break;
		case UIEvent::PHASE_DRAG:
		{
			MouseInputDrag(event);
		}break;
		case UIEvent::PHASE_MOVE:
		{
			ScreenWrapper::Instance()->SetCursor(event->point, GetCursor(event->point));
            RulerController::Instance()->UpdateRulerMarkers(event->point);
		}break;
		case UIEvent::PHASE_ENDED:
		{
			MouseInputEnd(event);
		}break;
		case UIEvent::PHASE_KEYCHAR:
		{
			KeyboardInput(event);
		}break;
	}
}

bool DefaultScreen::SystemInput(UIEvent *currentInput)
{
    if (PreviewController::Instance()->IsPreviewEnabled())
    {
        return UIScreen::SystemInput(PreprocessEventForPreview(currentInput));
    }

	Input(currentInput);
	return true;
}

HierarchyTreeControlNode* DefaultScreen::GetSelectedControl(const Vector2& point, const HierarchyTreeNode* parent) const
{
	if (!parent)
		return NULL;
	
	HierarchyTreeControlNode* selectedNode = NULL;
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& items = parent->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
		if (controlNode)
		{
			UIControl* control = controlNode->GetUIObject();
			if (control &&  control->IsPointInside(point))
				selectedNode = controlNode;
		}
		
		HierarchyTreeControlNode* childSelectNode = GetSelectedControl(point, node);
		if (childSelectNode)
			selectedNode = childSelectNode;
		
		if (selectedNode)
			return selectedNode;
	}
	
	return NULL;
}

DefaultScreen::SmartSelection::childsSet::~childsSet()
{
	for (std::vector<SmartSelection *>::iterator iter = begin();
		 iter != end();
		 ++iter)
	{
		SmartSelection* item = (*iter);
		SAFE_DELETE(item);
	}
}

DefaultScreen::SmartSelection::SmartSelection(HierarchyTreeNode::HIERARCHYTREENODEID id)
{
	//child = NULL;
	this->id = id;
	parent = NULL;
}

bool DefaultScreen::SmartSelection::IsEqual(const SmartSelection* item) const
{
	if (this->id != item->id)
		return false;

	if (this->childs.size() != item->childs.size())
		return false;
	
	DefaultScreen::SmartSelection::childsSet::const_iterator leftIter;
	for (leftIter = this->childs.begin();
		 leftIter != this->childs.end();
		 ++leftIter)
	{
		const DefaultScreen::SmartSelection* left = (*leftIter);
		const DefaultScreen::SmartSelection* right = NULL;
		for (DefaultScreen::SmartSelection::childsSet::const_iterator iter = item->childs.begin();
			 iter != item->childs.end();
			 ++iter)
		{
			if ((*iter)->id == left->id)
			{
				right = (*iter);
			}
		}
		if (!right)
			return false;
		
		if (!left->IsEqual(right))
			return false;
	}
	return true;
}

HierarchyTreeNode::HIERARCHYTREENODEID DefaultScreen::SmartSelection::GetFirst() const
{
	if (childs.size())
		return (*childs.begin())->GetFirst();

	return this->id;
}

HierarchyTreeNode::HIERARCHYTREENODEID DefaultScreen::SmartSelection::GetLast() const
{
	SelectionVector selection;
	FormatSelectionVector(selection);
	// The last element in selection array is on the top - so we should get it
	if (selection.size() > 0)
	{
		return selection[selection.size() - 1];
	}

	return this->id;
}

DefaultScreen::SmartSelection::SelectionVector DefaultScreen::SmartSelection::GetAll() const
{
	SelectionVector selection;
	FormatSelectionVector(selection);
	
	return selection;
}

void DefaultScreen::SmartSelection::FormatSelectionVector(SelectionVector &selection) const
{
	for (childsSet::const_iterator iter = childs.begin();
		 iter != childs.end();
		 ++iter)
	{
		const SmartSelection* item = (*iter);
		// Add item id to set before recursive call.
		// This will prevent adding value to array in reverse order (for controls with childs)
		selection.push_back(item->id);
		
		item->FormatSelectionVector(selection);
	}
}

HierarchyTreeNode::HIERARCHYTREENODEID DefaultScreen::SmartSelection::GetNext(HierarchyTreeNode::HIERARCHYTREENODEID id) const
{
	SelectionVector selection;
	FormatSelectionVector(selection);

	bool peakNext = false;
	// From top control to bottom control
	for (int32 i = selection.size() - 1; i >= 0; i--)
	{
		if (peakNext)
			return selection[i];
		
		if (selection[i] == id)
			peakNext = true;
	}
	return HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
}

void DefaultScreen::SmartGetSelectedControl(SmartSelection* list, const HierarchyTreeNode* parent, const Vector2& point) const
{
	if (!parent)
		return;
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& items = parent->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
		if (!controlNode)
			continue;

		UIControl* control = controlNode->GetUIObject();
		if (!control)
			continue;

		// Control can be selected if at least its subcontrol is visible (see pls DF-2420).
		if (!IsControlVisible(control))
		{
			continue;
		}

		if (!control->GetVisibleForUIEditor())
		{
			continue;
		}

		if (control->IsPointInside(point))
		{
			SmartSelection* newList = new SmartSelection(node->GetId());
			list->childs.push_back(newList);
			SmartGetSelectedControl(newList, node, point);
		}
		else
		{
			SmartGetSelectedControl(list, node, point);
		}
	}
}

HierarchyTreeControlNode* DefaultScreen::GetSelectedControl(const Vector2& point)
{
	const HierarchyTreeController::SELECTEDCONTROLNODES &selectedNodes = HierarchyTreeController::Instance()->GetActiveControlNodes();
	for (HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = selectedNodes.begin();
		 iter != selectedNodes.end();
		 ++iter)
	{
		HierarchyTreeControlNode* control = (*iter);
        if (!control || !control->GetUIObject())
        {
            continue;
        }

		if (control->GetUIObject()->IsPointInside(point) || (GetResizeType(control, point) != ResizeTypeNoResize))
		{
			return control;
		}
	}
	return NULL;
}

HierarchyTreeControlNode* DefaultScreen::SmartGetSelectedControl(const Vector2& point) const
{
	SmartSelection root(HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY);
	SmartGetSelectedControl(&root, HierarchyTreeController::Instance()->GetActiveScreen(), point);
	// DF-1593 - Always select first control (on top)
	return dynamic_cast<HierarchyTreeControlNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(root.GetLast()));
}

void DefaultScreen::GetSelectedControl(HierarchyTreeNode::HIERARCHYTREENODESLIST& list, const Rect& rect, const HierarchyTreeNode* parent) const
{
	if (!parent)
		return;
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& items = parent->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
		if (!controlNode)
			continue;
		
		UIControl* control = controlNode->GetUIObject();
		if (!control->GetVisible())
			continue;
        
        if (!control->GetVisibleForUIEditor())
			continue;
		
		Rect controlRect = GetControlRect(controlNode);
		if (controlRect.RectIntersects(rect))
			list.push_back(node);
		
		GetSelectedControl(list, rect, node);
	}
}

HierarchyTreeController::SELECTEDCONTROLNODES DefaultScreen::GetActiveMoveControls() const
{
	HierarchyTreeController::SELECTEDCONTROLNODES list = HierarchyTreeController::Instance()->GetActiveControlNodes();
	
	//YZ we need apply only for parent object
	HierarchyTreeController::SELECTEDCONTROLNODES selectedList;
	HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter;
	for (iter = list.begin(); iter != list.end(); ++iter)
	{
		HierarchyTreeControlNode* controlNode = (*iter);
		HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter2;
		bool isChild = false;
		for (iter2 = list.begin(); iter2 != list.end(); ++iter2)
		{
			HierarchyTreeControlNode* controlNode2 = (*iter2);
			if (controlNode2->IsHasChild(controlNode))
			{
				isChild = true;
				break;
			}
		}
		
		if (!isChild)
		{
			if (std::find(selectedList.begin(), selectedList.end(), controlNode) == selectedList.end())
			{
				selectedList.push_back(controlNode);
			}
		}
	}

	return selectedList;
}

void DefaultScreen::ApplyMoveDelta(const Vector2& delta)
{
	const HierarchyTreeController::SELECTEDCONTROLNODES& selectedList = GetActiveMoveControls();
	
	HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter;
	for (iter = selectedList.begin(); iter != selectedList.end(); ++iter)
	{
		HierarchyTreeControlNode* controlNode = (*iter);
		UIControl* control = controlNode->GetUIObject();
		if (control)
		{
			
			control->SetPosition(startControlPos[control] + delta);
		}
	}
}

void DefaultScreen::ResetMoveDelta()
{
	ApplyMoveDelta(Vector2(0, 0));
}

void DefaultScreen::SaveControlsPostion()
{
	startControlPos.clear();
	const HierarchyTreeController::SELECTEDCONTROLNODES& selectedList = HierarchyTreeController::Instance()->GetActiveControlNodes();
	HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter;
	for (iter = selectedList.begin(); iter != selectedList.end(); ++iter)
	{
		HierarchyTreeControlNode* controlNode = (*iter);
		UIControl* control = controlNode->GetUIObject();
		if (control)
		{
			startControlPos[control] = control->GetPosition();
		}
	}
}

void DefaultScreen::MoveControl(const Vector2& delta)
{
	ControlsMoveCommand* cmd = new ControlsMoveCommand(GetActiveMoveControls(), delta);
	CommandsController::Instance()->ExecuteCommand(cmd);
	SafeRelease(cmd);
}

void DefaultScreen::DeleteSelectedControls()
{
	const HierarchyTreeController::SELECTEDCONTROLNODES& selectedControls = HierarchyTreeController::Instance()->GetActiveControlNodes();
	HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter;
	HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator innerIter;
	HierarchyTreeNode::HIERARCHYTREENODESLIST nodes;
	HierarchyTreeController::SELECTEDCONTROLNODES parentNodes(selectedControls);
	
	// DF-1273 - remove all child nodes of selected controls - we don't have to remove them here
	for (iter = selectedControls.begin(); iter != selectedControls.end(); ++iter)
	{
		HierarchyTreeNode *node = (*iter);
		for (innerIter = selectedControls.begin(); innerIter != selectedControls.end(); ++innerIter)
		{			
			if (node->IsHasChild(*innerIter))
			{
				HierarchyTreeController::SELECTEDCONTROLNODES::iterator parentNodeIter =
					std::find(parentNodes.begin(), parentNodes.end(), *innerIter);
				if (parentNodeIter != parentNodes.end())
				{
					parentNodes.erase(parentNodeIter);
				}
			}
		}
	}

	// DF-1273 - put only "parent" nodes to delete
	for (iter = parentNodes.begin(); iter != parentNodes.end(); ++iter)
		nodes.push_back(*iter);

	if (!nodes.size())
		return;
	
	DeleteSelectedNodeCommand* cmd = new DeleteSelectedNodeCommand(nodes);
	CommandsController::Instance()->ExecuteCommand(cmd);
	SafeRelease(cmd);
}

Qt::CursorShape DefaultScreen::GetCursor(const Vector2& point)
{
	if (inputState == InputStateScreenMove)
		return Qt::OpenHandCursor;
		
	if (inputState == InputStateSize)
		return ResizeTypeToQt(resizeType, lastSelectedControl);
	
	Vector2 pos = LocalToInternal(point);
	
	Qt::CursorShape cursor = Qt::ArrowCursor;

	const HierarchyTreeController::SELECTEDCONTROLNODES& selectedControls = HierarchyTreeController::Instance()->GetActiveControlNodes();
	HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter;
	for (iter = selectedControls.begin(); iter != selectedControls.end(); ++iter)
	{
		//nodes.insert(*iter);
		const HierarchyTreeControlNode* node = (*iter);
		if (!node)
			continue;
		
		cursor = Qt::SizeAllCursor;
		
		ResizeType resize = GetResizeType(node, pos);
		if (resize == ResizeTypeNoResize)
			continue;
		
		return ResizeTypeToQt(resize, node);
	}

	return cursor;
}

ResizeType DefaultScreen::GetResizeType(const HierarchyTreeControlNode* selectedControlNode, const Vector2& pos) const
{
	UIControl* selectedControl = selectedControlNode->GetUIObject();
	if (!selectedControl)
	{
		return ResizeTypeNoResize;
	}
	 
	//check is resize
	bool horLeft = false;
	bool horRight = false;
	bool verTop = false;
	bool verBottom = false;
    
    Vector4 distancesToBounds = CalculateDistancesToControlBounds(selectedControl, pos);
    if (abs(distancesToBounds.x) < SIZE_CURSOR_DELTA)
    {
        verTop = true;
    }
    if (abs(distancesToBounds.y) < SIZE_CURSOR_DELTA)
    {
        horRight = true;
    }
    if (abs(distancesToBounds.z) < SIZE_CURSOR_DELTA)
    {
        verBottom = true;
    }
    if (abs(distancesToBounds.w) < SIZE_CURSOR_DELTA)
    {
        horLeft = true;
    }

    // If at least one coord is less than zero and more than SIZE_CURSOR_DELTA - we are outside the control.
    if ((distancesToBounds.x < 0 || distancesToBounds.y < 0 || distancesToBounds.z < 0 || distancesToBounds.w < 0) &&
        (verTop || verBottom || horLeft || horRight))
    {
        return ResizeTypeNoResize;
    }

	if (horLeft && verTop)
		return ResizeTypeLeftTop;
	if (horRight && verBottom)
		return ResizeTypeRightBottom;
	if (horLeft && verBottom)
		return ResizeTypeLeftBottom;
	if (horRight && verTop)
		return ResizeTypeRigthTop;
	if (horLeft)
		return ResizeTypeLeft;
	if (horRight)
		return ResizeTypeRight;
	if (verTop)
		return ResizeTypeTop;
	if (verBottom)
		return ResizeTypeBottom;
	
	return ResizeTypeNoResize;
}

Qt::CursorShape DefaultScreen::ResizeTypeToQt(ResizeType resize, const HierarchyTreeControlNode* selectedNode)
{
    if (!selectedNode || !selectedNode->GetUIObject())
    {
        return Qt::ArrowCursor;
    }

    ResizeType rotatedResizeType = UIControlResizeHelper::GetRotatedResizeType(resize, selectedNode->GetUIObject());

	if (rotatedResizeType == ResizeTypeLeftTop || rotatedResizeType == ResizeTypeRightBottom)
		return Qt::SizeFDiagCursor;
	if (rotatedResizeType == ResizeTypeRigthTop || rotatedResizeType == ResizeTypeLeftBottom)
		return Qt::SizeBDiagCursor;
	if (rotatedResizeType == ResizeTypeLeft || rotatedResizeType == ResizeTypeRight)
		return Qt::SizeHorCursor;
	if (rotatedResizeType == ResizeTypeTop || rotatedResizeType == ResizeTypeBottom)
		return Qt::SizeVerCursor;

	return Qt::ArrowCursor;
}

bool DefaultScreen::IsPointInsideControlWithDelta(UIControl* uiControl, const Vector2& point, int32 pointDelta) const
{
    if (!uiControl)
    {
        return false;
    }

    Vector4 distancesToBounds = CalculateDistancesToControlBounds(uiControl, point);
    return (distancesToBounds.x > pointDelta && distancesToBounds.y > pointDelta &&
            distancesToBounds.z > pointDelta && distancesToBounds.w > pointDelta);
}

Vector4 DefaultScreen::CalculateDistancesToControlBounds(UIControl* uiControl, const Vector2& point) const
{
    Vector4 resultVector;

    if (!uiControl)
    {
        return resultVector;
    }

    // Convert control's rect to polygon taking rotation into account.
    const UIGeometricData &gd = uiControl->GetGeometricData();
    Polygon2 poly;
    gd.GetPolygon(poly);
    
    const Vector2* polygonPoints = poly.GetPoints();
    
    // Distances are build in the following order: top, left, right, bottom.
    resultVector.x = Collisions::Instance()->CalculateDistanceFrom2DPointTo2DLine(polygonPoints[0], polygonPoints[1], point);
    resultVector.y = Collisions::Instance()->CalculateDistanceFrom2DPointTo2DLine(polygonPoints[1], polygonPoints[2], point);
    resultVector.z = Collisions::Instance()->CalculateDistanceFrom2DPointTo2DLine(polygonPoints[2], polygonPoints[3], point);
    resultVector.w = Collisions::Instance()->CalculateDistanceFrom2DPointTo2DLine(polygonPoints[3], polygonPoints[0], point);

    return resultVector;
}

void DefaultScreen::ApplySizeDelta(const Vector2& delta)
{
	if (!lastSelectedControl || !lastSelectedControl->GetUIObject())
	{
		return;
	}

	// The helper will calculate both resize (taking rotation into account) and clamp.
    UIControlResizeHelper::ResizeControl(resizeType, lastSelectedControl->GetUIObject(), resizeRect,  delta);
}

void DefaultScreen::ResetSizeDelta()
{
	if (!lastSelectedControl)
		return;
	
	lastSelectedControl->GetUIObject()->SetRect(resizeRect);
}

void DefaultScreen::ResizeControl()
{
	if (!IsNeedApplyResize())
		return;
	
	if (!lastSelectedControl)
		return;
	
	Rect rect = lastSelectedControl->GetUIObject()->GetRect();

	ResetSizeDelta();
	ControlResizeCommand* cmd = new ControlResizeCommand(lastSelectedControl->GetId(), resizeRect, rect);
	CommandsController::Instance()->ExecuteCommand(cmd);
	SafeRelease(cmd);
}

bool DefaultScreen::IsNeedApplyResize() const
{
	if (!lastSelectedControl)
		return false;

	Rect rect = lastSelectedControl->GetUIObject()->GetRect();
	return rect != resizeRect;
}

void DefaultScreen::ApplyMouseSelection(const Vector2& rectSize)
{
	Rect rect = selectorControl->GetRect();
	rect.dx = rectSize.x;
	rect.dy = rectSize.y;
	selectorControl->SetRect(rect);
	if (rect.dx < 0)
	{
		rect.x += rect.dx;
		rect.dx = -rect.dx;
	}
	if (rect.dy < 0)
	{
		rect.y += rect.dy;
		rect.dy = -rect.dy;
	}
	
	//update selecetion
	HierarchyTreeController::SELECTEDCONTROLNODES oldNodes = HierarchyTreeController::Instance()->GetActiveControlNodes();
	HierarchyTreeNode::HIERARCHYTREENODESLIST newNodes;
	GetSelectedControl(newNodes, rect, HierarchyTreeController::Instance()->GetActiveScreen());
	
	for(HierarchyTreeController::SELECTEDCONTROLNODES::iterator iter = oldNodes.begin(); iter != oldNodes.end(); ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
        
        bool controlNodeFound = (controlNode && std::find(newNodes.begin(), newNodes.end(), controlNode) != newNodes.end());
        //HierarchyTreeNode* listNode = (*iter);
		if (!controlNodeFound)
			HierarchyTreeController::Instance()->UnselectControl(controlNode);
	}
	
	for(HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = newNodes.begin(); iter != newNodes.end(); ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
		if (controlNode && std::find(oldNodes.begin(), oldNodes.end(), controlNode) == oldNodes.end())
		{
			HierarchyTreeController::Instance()->SelectControl(controlNode);
		}
	}
}

void DefaultScreen::MouseInputBegin(const DAVA::UIEvent* event)
{
	if (this->mouseAlreadyPressed)
	{
		// No need to handle Multiple Mouse Input Begins - it causes strange issues under Windows.
		return;
	}
	this->mouseAlreadyPressed = true;

	if (event->tid == UIEvent::BUTTON_1 && CheckEnterScreenMoveState())
	{
		return;
	}

	if (inputState == InputStateScreenMove)
		return;
	
	lastSelectedControl = NULL;
	useMouseUpSelection = true;
	
	Vector2 localPoint = event->point;
	Vector2 point = LocalToInternal(localPoint);

	HierarchyTreeControlNode* selectedControlNode = GetSelectedControl(point);
	
	if (!selectedControlNode)
	{
		selectedControlNode = SmartGetSelectedControl(point);
		//If smart selection was used - we don't need additional mouseUp selection
		useMouseUpSelection = false;
	}
	
	while (selectedControlNode)
	{
		UIControl* selectedControl = selectedControlNode->GetUIObject();
		if (!selectedControl)
		{
			selectedControlNode = NULL;
			break;
		}

		ResizeType resize = GetResizeType(selectedControlNode, point);
		if (resize != ResizeTypeNoResize &&
			HierarchyTreeController::Instance()->IsControlSelected(selectedControlNode))
		{
			resizeType = resize;
			lastSelectedControl = selectedControlNode;
			resizeRect = selectedControl->GetRect();
			inputState = InputStateSize;
		}
		else
		{
			if (HierarchyTreeController::Instance()->IsControlSelected(selectedControlNode))
			{
				//Don't check active controls size anymore - we have to be able to deselect any control
				if (/*HierarchyTreeController::Instance()->GetActiveControlNodes().size() > 1 &&*/
					InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_SHIFT))
				{
					lastSelectedControl = selectedControlNode;
					//If controls was selected with SHIFT key pressed - we don't need mouseUp selection
					useMouseUpSelection = false;
				}
			}
			else
			{
				if (!InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_SHIFT))
					HierarchyTreeController::Instance()->ResetSelectedControl();
					
				HierarchyTreeController::Instance()->SelectControl(selectedControlNode);
			}
			inputState = InputStateSelection;
			SaveControlsPostion();
		}
		
		break;
	}
		
	inputPos = event->point;
	if (!selectedControlNode)
	{
		HierarchyTreeController::Instance()->ResetSelectedControl();
		inputState = InputStateSelectorControl;
		selectorControl->SetRect(Rect(point.x, point.y, 0, 0));
	}
}

void DefaultScreen::CopySelectedControls()
{
	//Set copy flag
	copyControlsInProcess = true;
	HierarchyTreeNode* parentConrol = NULL;
	//Get current selected controls on screen
	const HierarchyTreeController::SELECTEDCONTROLNODES &selectedNodes = HierarchyTreeController::Instance()->GetActiveControlNodes();

    // Need to check whether we have at least one subcontrol and disable copying in this case.
    // See please DF-2684 for details.
    if (CopyPasteController::Instance()->SubcontrolsSelected(selectedNodes))
    {
        return;
    }

	//Get firt parent control from list of selected controls
	for (HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = selectedNodes.begin();
		 iter != selectedNodes.end();
		 ++iter)
	{
		HierarchyTreeControlNode* control = (*iter);
		if (!control)
		{
			continue;
		}
		//skip child control when copy parent
		if (CopyPasteController::Instance()->ControlIsChild(selectedNodes, control))
		{
			continue;
		}

		//Set parent control to which will be copied all selected items
		parentConrol = control->GetParent();
		break;
	}
	
	if (parentConrol)
	{	//Copy selected items
		CopyPasteController::Instance()->CopyControls(selectedNodes);
		CopyPasteController::Instance()->Paste(parentConrol);
		SaveControlsPostion();
	}
}

void DefaultScreen::MouseInputDrag(const DAVA::UIEvent* event)
{
	if (!this->mouseAlreadyPressed)
	{
		// If the Mouse Button isn't pressed - ignore drag messages, there may be fake ones.
		return;
	}

	HandleScreenMove(event);

	Vector2 delta = GetInputDelta(event->point);
	
	if (inputState == InputStateSelection)
	{
		if ((inputPos - event->point).Length() > MIN_DRAG_DELTA)
		{
			inputState = InputStateDrag;
		}
	}
	
	if (inputState == InputStateDrag)
	{
		//If control key is pressed - we are going to copy control(s)
		if (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_CTRL) && !copyControlsInProcess)
		{
			CopySelectedControls();
		}	
	
		ApplyMoveDelta(delta);
	}
	
	if (inputState == InputStateSize)
	{
		ApplySizeDelta(delta);
	}
	
	if (inputState == InputStateSelectorControl)
	{
		ApplyMouseSelection(delta);
	}
}

void DefaultScreen::MouseInputEnd(const DAVA::UIEvent* event)
{
	// Mouse input is finished.
	this->mouseAlreadyPressed = false;

	//Always reset copy flag
	copyControlsInProcess = false;
	if (inputState == InputStateDrag)
	{
		Vector2 delta = GetInputDelta(event->point);
		ResetMoveDelta();
		MoveControl(delta);
		startControlPos.clear();

	}
	
	if (inputState == InputStateSize)
	{
		ResizeControl();
	}
	
	if (lastSelectedControl && inputState == InputStateSelection)
	{
		HierarchyTreeController::Instance()->ChangeItemSelection(lastSelectedControl);
	}
	
	//Use additional control selection. This will allow to scroll through multiple contols
	//located in the same area
	bool bResetControlPosition = true;
	if ((inputState == InputStateSelection) && useMouseUpSelection)
	{		
		Vector2 localPoint = event->point;
		Vector2 point = LocalToInternal(localPoint);
			
		switch(event->tid)
		{
			case UIEvent::BUTTON_1: // For left mouse button we use standard selection
				HandleMouseLeftButtonClick(point);
				break;
			case UIEvent::BUTTON_2: // For right mouse button we should show context menu with avaiable controls at current point
			{
				HandleMouseRightButtonClick(point);
				bResetControlPosition = false;
				break;
			}
			default: // Do nothing for other buttons
				break;
		}
	}
	
	CheckExitScreenMoveState();

	// We don't have to reset control for right mouse click. Control position is not changed and we don't select another control
	// So we have to keep previous "selection" for that case
	if (bResetControlPosition)
	{
		lastSelectedControl = NULL;
		inputState = InputStateSelection;
		startControlPos.clear();
	}
}

void DefaultScreen::HandleMouseRightButtonClick(const Vector2& point)
{
	SmartSelection selection(HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY);
	SmartGetSelectedControl(&selection, HierarchyTreeController::Instance()->GetActiveScreen(), point);
	ShowControlContextMenu(selection);
}

void DefaultScreen::HandleMouseLeftButtonClick(const Vector2& point)
{
	HierarchyTreeControlNode* selectedControlNode = SmartGetSelectedControl(point);
	if (selectedControlNode)
	{
		if (!HierarchyTreeController::Instance()->IsControlSelected(selectedControlNode))
		{
			if (!InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_SHIFT))
				HierarchyTreeController::Instance()->ResetSelectedControl();

				HierarchyTreeController::Instance()->SelectControl(selectedControlNode);
		}
		SaveControlsPostion();
	}
}

void DefaultScreen::ShowControlContextMenu(const SmartSelection& selection)
{
	QMenu menu;
	// Get the whole selection vector available
	const DefaultScreen::SmartSelection::SelectionVector selectionVector = selection.GetAll();
	for (int32 i = selectionVector.size() - 1; i >= 0; i--)
	{
		HierarchyTreeNode *node = HierarchyTreeController::Instance()->GetTree().GetNode(selectionVector[i]);
		HierarchyTreeControlNode *controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
			
		if (!controlNode)
			continue;
		// Create menu action and put nodeId to it as property
		QAction* controlAction = new QAction(controlNode->GetName(), &menu);
		controlAction->setProperty(MENU_PROPERTY_ID, selectionVector[i]);
		menu.addAction(controlAction);
	}
	
  	connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(ControlContextMenuTriggered(QAction*)));
	menu.exec(QCursor::pos());
}

void DefaultScreen::ControlContextMenuTriggered(QAction* action)
{
	// Get controlNode for specific action triggered
	int nodeId = action->property(MENU_PROPERTY_ID).toInt();
	HierarchyTreeNode *node = HierarchyTreeController::Instance()->GetTree().GetNode(nodeId);
	HierarchyTreeControlNode *controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
			
	if (controlNode)
	{
		if (!HierarchyTreeController::Instance()->IsControlSelected(controlNode))
		{
			HierarchyTreeController::Instance()->ResetSelectedControl();
			HierarchyTreeController::Instance()->SelectControl(controlNode);			
		}
		
		SaveControlsPostion();
	}
}

void DefaultScreen::KeyboardInput(const DAVA::UIEvent* event)
{
	switch (event->tid)
	{
		case DVKEY_ESCAPE:
		{
			if (inputState == InputStateDrag)
			{
				ResetMoveDelta();
				inputState = InputStateSelection;
			}
			if (inputState == InputStateSize)
			{
				ResetSizeDelta();
				inputState = InputStateSelection;
			}
		}break;
		case DVKEY_UP:
		{
			MoveControl(Vector2(0, -GetControlMoveDelta()));
		}break;
		case DVKEY_DOWN:
		{
			MoveControl(Vector2(0, GetControlMoveDelta()));
		}break;
		case DVKEY_LEFT:
		{
			MoveControl(Vector2(-GetControlMoveDelta(), 0));
		}break;
		case DVKEY_RIGHT:
		{
			MoveControl(Vector2(GetControlMoveDelta(), 0));
		}break;
		case DVKEY_DELETE:
		case DVKEY_BACKSPACE:
		{
			DeleteSelectedControls();
		}break;
		case DVKEY_C:
		{
			if (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_CTRL))
			{
				CopyPasteController::Instance()->CopyControls(HierarchyTreeController::Instance()->GetActiveControlNodes());
			}
		}break;
		case DVKEY_V:
		{
			if (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_CTRL))
			{
                HierarchyTreeController::SELECTEDCONTROLNODES selectedList = HierarchyTreeController::Instance()->GetActiveControlNodes();                
                if (selectedList.size() == 0)
                {
                    // No controls selected - paste to screen.
                    HierarchyTreeScreenNode* activeScreenNode = HierarchyTreeController::Instance()->GetActiveScreen();
                    if (activeScreenNode)
                    {
                        CopyPasteController::Instance()->Paste(activeScreenNode);
                        break;
                    }
                }

                // Get the first non-null control node and paste to it.
                HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter;
                for (iter = selectedList.begin(); iter != selectedList.end(); ++iter)
                {
                    HierarchyTreeControlNode* controlNode = (*iter);
                    if (controlNode)
                    {
                        CopyPasteController::Instance()->Paste(controlNode);
                        break;
                    }
                }
  			}		
            break;
        }
            
		default:
			break;
	}
	
}

Vector2 DefaultScreen::GetInputDelta(const Vector2& point, bool applyScale) const
{
	Vector2 delta = GridController::Instance()->RecalculateMousePos(point - inputPos);

	if (applyScale)
	{
		delta.x /= scale.x;
		delta.y /= scale.y;
	}

	return delta;
}

void DefaultScreen::BacklightControl(const Vector2& position)
{
	Vector2 pos = LocalToInternal(position);
	
	HierarchyTreeControlNode* newSelectedNode = SmartGetSelectedControl(pos);
	if (newSelectedNode)
	{
		if (!HierarchyTreeController::Instance()->IsControlSelected(newSelectedNode))
		{
			HierarchyTreeController::Instance()->ResetSelectedControl();
			HierarchyTreeController::Instance()->SelectControl(newSelectedNode);
		}
	}
	else if (HierarchyTreeController::Instance()->GetActiveControlNodes().size())
	{
		HierarchyTreeController::Instance()->ResetSelectedControl();
	}
}

bool DefaultScreen::IsDropEnable(const DAVA::Vector2 &position) const
{
	Vector2 pos = LocalToInternal(position);
	HierarchyTreeAggregatorControlNode* newSelectedNode = dynamic_cast<HierarchyTreeAggregatorControlNode*>(SmartGetSelectedControl(pos));
	if (newSelectedNode)
	{
		// Don't allow to drop anything to Aggregators.
		return false;
	}

	return true;
}

Rect DefaultScreen::GetControlRect(const HierarchyTreeControlNode* controlNode) const
{
	Rect rect;
	
	if (!controlNode)
		return rect;
	
	UIControl* control = controlNode->GetUIObject();
	if (!control)
		return rect;
	
	rect = control->GetRect();
	rect += controlNode->GetParentDelta(true);

	return rect;
}

bool DefaultScreen::CheckEnterScreenMoveState()
{
	// If we are here - the Mouse Down event just happened. Check whether we are
	// in appropriate state and the key is pressed.
	if (inputState == InputStateSelection && IsMoveScreenKeyPressed())
	{
		inputState = InputStateScreenMove;
		ScreenWrapper::Instance()->RequestUpdateCursor();
		inputPos = Vector2(-1, -1);

		return true;
	}
	
	return false;
}

void DefaultScreen::CheckScreenMoveState()
{
	// This is called on every frame. If the move sceeen key is released -
	// treat this as "reset Screen Move state".
	if (inputState == InputStateScreenMove && !IsMoveScreenKeyPressed())
	{
		inputState = InputStateSelection;
	}
}

void DefaultScreen::CheckExitScreenMoveState()
{
	// Reset in any case.
	if (inputState == InputStateScreenMove)
	{
		inputState = InputStateSelection;
		ScreenWrapper::Instance()->RequestUpdateCursor();
	}
}

bool DefaultScreen::IsMoveScreenKeyPressed()
{
	//return InputSystem::Instance()->GetKeyboard()->IsKeyPressed(MOVE_SCREEN_KEY);
	Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
	return (modifiers & Qt::AltModifier);
}

int32 DefaultScreen::GetControlMoveDelta()
{
	Qt::KeyboardModifiers modifiers = QApplication::queryKeyboardModifiers();
	return (modifiers & Qt::ShiftModifier) ? COARSE_CONTROL_MOVE_DELTA : FINE_CONTROL_MOVE_DELTA;
}

void DefaultScreen::HandleScreenMove(const DAVA::UIEvent* event)
{
	if (inputState == InputStateScreenMove)
	{
		const Vector2& pos = event->point;
		if (Abs(inputPos.x + 1) < 0.1f && (inputPos.y + 1) < 0.1f)
		{
			inputPos = pos;
		}

		// In this particular case don't take Scale into account.
		Vector2 delta = GetInputDelta(pos, false);
		ScreenWrapper::Instance()->RequestViewMove(-delta);
		inputPos = pos;
	}
}

bool DefaultScreen::IsControlVisible(UIControl* uiControl) const
{
	bool isVisible = false;
	IsControlVisibleRecursive(uiControl, isVisible);

	return isVisible;
}

void DefaultScreen::IsControlVisibleRecursive(const UIControl* uiControl, bool& isVisible) const
{
	if (!uiControl)
	{
		isVisible = false;
		return;
	}

	isVisible |= uiControl->GetVisible();

	const List<UIControl*>& children = uiControl->GetChildren();
	for(List<UIControl*>::const_iterator iter = children.begin(); iter != children.end(); iter ++)
	{
		IsControlVisibleRecursive(*iter, isVisible);
	}
}

void DefaultScreen::SetScreenControl(ScreenControl* control)
{
    screenControl = control;
}

UIEvent* DefaultScreen::PreprocessEventForPreview(UIEvent* event)
{
    switch (event->phase)
    {
        case UIEvent::PHASE_BEGAN:
        case UIEvent::PHASE_DRAG:
        case UIEvent::PHASE_MOVE:
        case UIEvent::PHASE_ENDED:
        {
            event->point = LocalToInternal(event->point);
            break;
        }

        default:
        {
            break;
        }
    }

    return event;
}
