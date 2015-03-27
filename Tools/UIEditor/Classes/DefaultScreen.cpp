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

#include "CommandsController.h"
#include "ControlCommands.h"
#include "ItemsCommand.h"
#include "GuideCommands.h"

#include "CopyPasteController.h"
#include "HierarchyTreeAggregatorControlNode.h"

#include "Ruler/RulerController.h"
#include "Guides/GuidesEnums.h"

#include "PreviewController.h"

#include <QMenu>
#include <QAction>
#include <QApplication>

#define SIZE_CURSOR_DELTA 5
#define MIN_DRAG_DELTA 3

// Coarse/Fine Control Move delta.
#define COARSE_CONTROL_MOVE_DELTA 10
#define FINE_CONTROL_MOVE_DELTA 1

// Coarse/Fine Guides Move delta.
#define COARSE_GUIDE_MOVE_DELTA 10
#define FINE_GUIDE_MOVE_DELTA 1

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
    
    isStickedToX = false;
    isStickedToY = false;
    
    isNeedHandleScreenScaleChanged = false;
    isNeedHandleScreenPositionChanged = false;
}

DefaultScreen::~DefaultScreen()
{
	SafeRelease(selectorControl);
}

void DefaultScreen::Update(float32 /*timeElapsed*/)
{
	CheckScreenMoveState();
    
    // Handle the "screen scale changed"/"screen position changed" after the scale/translate is set.
    HandleScreenScalePositionChanged();
}

void DefaultScreen::Draw(const UIGeometricData &geometricData)
{
	UIScreen::Draw(geometricData);
}

void DefaultScreen::SystemDraw(const UIGeometricData &geometricData)
{
    bool previewEnabled = PreviewController::Instance()->IsPreviewEnabled();
    Color oldColor = RenderManager::Instance()->GetColor();

    Matrix4 wt = Matrix4::MakeTranslation(Vector3(pos)) * Matrix4::MakeScale(Vector3(scale.x, scale.y, 1.f));
    RenderManager::SetDynamicParam(PARAM_VIEW, &wt, UPDATE_SEMANTIC_ALWAYS);

    RenderManager::Instance()->SetColor(ScreenWrapper::Instance()->GetBackgroundFrameColor());
    RenderHelper::Instance()->FillRect(ScreenWrapper::Instance()->GetBackgroundFrameRect(), RenderState::RENDERSTATE_2D_BLEND);
    RenderManager::Instance()->SetColor(oldColor);

   // For Preview mode display only what is inside the preview rectangle.
    if (previewEnabled)
    {
        RenderSystem2D::Instance()->PushClip();
        
        Rect previewClipRect;
        previewClipRect.SetSize(PreviewController::Instance()->GetTransformData().screenSize);
        RenderSystem2D::Instance()->SetClip(previewClipRect);
    }

	UIScreen::SystemDraw(geometricData);

    if (previewEnabled)
    {
        RenderSystem2D::Instance()->PopClip();
    }
    else if (inputState == InputStateSelectorControl)
    {
		selectorControl->SystemDraw(geometricData);
    }
    
    if (!previewEnabled)
    {
        DrawGuides();
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
	if (!childs.empty())
		return childs.front()->GetFirst();

	return id;
}

HierarchyTreeNode::HIERARCHYTREENODEID DefaultScreen::SmartSelection::GetLast() const
{
    if (!childs.empty())
        return childs.back()->GetLast();

    return id;
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

		if (!control->GetVisible())
			continue;

        SmartSelection* currList = list;
        bool controlVisible = IsControlVisible(control);

        if (controlVisible && control->IsPointInside(point))
        {
            SmartSelection* newList = new SmartSelection(node->GetId());
            list->childs.push_back(newList);
            currList = newList;
        }

        if (controlVisible)
        {
            SmartGetSelectedControl(currList, node, point);
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
        if (!control)
            continue;

        if (!control->GetVisible())
            continue;

        bool controlVisible = IsControlVisible(control);

        if (controlVisible)
        {
            Rect controlRect = GetControlRect(controlNode, true);
            if (controlRect.RectIntersects(rect))
                list.push_back(node);
        }

        if (controlVisible)
        {
            GetSelectedControl(list, rect, node);
        }
	}
}

HierarchyTreeController::SELECTEDCONTROLNODES DefaultScreen::GetActiveMoveControls() const
{
	const HierarchyTreeController::SELECTEDCONTROLNODES &list = HierarchyTreeController::Instance()->GetActiveControlNodes();
	
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
            float32 parentsTotalAngle = control->GetParent()->GetGeometricData().angle;
            if(parentsTotalAngle != 0)
            {
                Matrix3 tmp;
                tmp.BuildRotation(-parentsTotalAngle);
                Vector2 rotatedVec = delta * tmp;
                control->SetPosition(startControlPos[control] + rotatedVec);
            }
            else
            {
                control->SetPosition(startControlPos[control] + delta);
            }
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

void DefaultScreen::DoKeyboardMove(eKeyboardMoveDirection moveDirection)
{
    // In case guides are selected - move guides, otherwise control.
    HierarchyTreeScreenNode* screenNode = HierarchyTreeController::Instance()->GetActiveScreen();
    if (!screenNode)
    {
        return;
    }

    bool moveGuides = screenNode && screenNode->AreGuidesSelected();
    
    int32 treshold = moveGuides ? GetGuideMoveDelta() : GetControlMoveDelta();
    Vector2 delta;
    
    switch (moveDirection)
    {
        case moveUp:
        {
            delta = Vector2 (0, -treshold);
            break;
        }
            
        case moveDown:
        {
            delta = Vector2(0, treshold);
            break;
        }
            
        case moveLeft:
        {
            delta = Vector2(-treshold, 0);
            break;
        }
        
        case moveRight:
        {
            delta = Vector2(treshold, 0);
            break;
        }
            
        default:
        {
            DVASSERT(false);
            break;
        }
    }
    
    if (moveGuides)
    {
        MoveGuides(moveDirection, delta);
    }
    else
    {
        MoveControl(delta, true);
    }
}

void DefaultScreen::MoveGuides(eKeyboardMoveDirection moveDirection, const Vector2& delta)
{
    HierarchyTreeScreenNode* screenNode = HierarchyTreeController::Instance()->GetActiveScreen();
    DVASSERT(screenNode);

    Vector2 minGuidePos = Vector2(FLT_MAX, FLT_MAX);
    Vector2 maxGuidePos = Vector2(FLT_MIN, FLT_MIN);
    bool horzGuidesSelected = false;
    bool vertGuidesSelected = false;

    // Check whether move is possible.
    const List<GuideData*>& selectedGuides = screenNode->GetSelectedGuides();
    for (List<GuideData*>::const_iterator iter = selectedGuides.begin(); iter != selectedGuides.end(); iter ++)
    {
        GuideData* curGuideData =  *iter;
        const Vector2& curPos = curGuideData->GetPosition();
        if (curGuideData->GetType() == GuideData::Horizontal)
        {
            horzGuidesSelected = true;
        }
        if (curGuideData->GetType() == GuideData::Vertical)
        {
            vertGuidesSelected = true;
        }

        if (curPos.x < minGuidePos.x)
        {
            minGuidePos.x = curPos.x;
        }
        if (curPos.y < minGuidePos.y)
        {
            minGuidePos.y = curPos.y;
        }
        if (curPos.x > maxGuidePos.x)
        {
            maxGuidePos.x = curPos.x;
        }
        if (curPos.y > maxGuidePos.y)
        {
            maxGuidePos.y = curPos.y;
        }
    }
    
    // No way to move vertically if no horz guides selected.
    if ((moveDirection == moveUp || moveDirection == moveDown) && !horzGuidesSelected)
    {
        return;
    }

    // No way to move horisontally if no vert guides selected.
    if ((moveDirection == moveLeft || moveDirection == moveRight) && !vertGuidesSelected)
    {
        return;
    }

    // Check whether guides remain in the screen bounds after move.
    Rect screenRect = ScreenWrapper::Instance()->GetBackgroundFrameRect();
    if (((minGuidePos.x + delta.x) <= screenRect.x) || ((minGuidePos.y + delta.y) <= screenRect.y))
    {
        return;
    }
    
    if (((maxGuidePos.x + delta.x) >= (screenRect.x + screenRect.dx)) || ((maxGuidePos.y + delta.y) >= (screenRect.y + screenRect.dy)))
    {
        return;
    }

    // All is OK - can move.
    MoveGuideCommand* cmd = new MoveGuideCommand(screenNode, delta);
    CommandsController::Instance()->ExecuteCommand(cmd);
    SafeRelease(cmd);
}

void DefaultScreen::MoveControl(const Vector2& delta, bool alignControlToIntegerPos)
{
	ControlsMoveCommand* cmd = new ControlsMoveCommand(GetActiveMoveControls(), delta, alignControlToIntegerPos);
	CommandsController::Instance()->ExecuteCommand(cmd);
	SafeRelease(cmd);
}

void DefaultScreen::DeleteSelectedControls()
{
	const HierarchyTreeController::SELECTEDCONTROLNODES& selectedControls = HierarchyTreeController::Instance()->GetActiveControlNodes();
    if (selectedControls.empty())
    {
        return;
    }

    HierarchyTreeNode::HIERARCHYTREENODESLIST nodesList;
    for (HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = selectedControls.begin(); iter != selectedControls.end(); iter ++)
    {
        nodesList.push_back(*iter);
    }

    // HierarchyTreeWidget is subscribed to this signal and will handle deletion.
    emit DeleteNodes(nodesList);
}

void DefaultScreen::MoveGuide(HierarchyTreeScreenNode* screenNode)
{
    if (!screenNode->GetMoveGuide())
    {
        // Nothing to move.
        return;
    }

    MoveGuideByMouseCommand* command = new MoveGuideByMouseCommand(screenNode);
    CommandsController::Instance()->ExecuteCommand(command);
    SafeRelease(command);
}

void DefaultScreen::DeleteSelectedGuides(HierarchyTreeScreenNode* screenNode)
{
    DeleteGuidesCommand* command = new DeleteGuidesCommand(screenNode);
    CommandsController::Instance()->ExecuteCommand(command);
    SafeRelease(command);
    
    HierarchyTreeController::Instance()->ResetSelectedControl();
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

    // Check whether the cursor is out of the control but within SIZE_CURSOR_DELTA value.
    // Separate check for all sides to avoid side effects like "resize on X allowed while
    // cursor is in X bounds but out of Y bounds".
    if (((distancesToBounds.x < -SIZE_CURSOR_DELTA || distancesToBounds.z < -SIZE_CURSOR_DELTA) &&
         !(horRight && horLeft)) ||
        ((distancesToBounds.y < -SIZE_CURSOR_DELTA || distancesToBounds.w < -SIZE_CURSOR_DELTA) &&
         !(verTop && verBottom)))
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

    Vector2 localPoint = event->point;
	Vector2 point = LocalToInternal(localPoint);
    
	if (event->tid == UIEvent::BUTTON_1 && CheckEnterScreenMoveState())
	{
		return;
	}

    HierarchyTreeScreenNode* screenNode = HierarchyTreeController::Instance()->GetActiveScreen();
	if (screenNode && event->tid == UIEvent::BUTTON_1 && screenNode->AreGuidesEnabled() && screenNode->StartMoveGuide(point))
	{
		inputState = InputStateGuideMove;
        HierarchyTreeController::Instance()->ResetSelectedControl();
        return;
	}

	if (inputState == InputStateScreenMove)
		return;
	
	lastSelectedControl = NULL;
	useMouseUpSelection = true;
	

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
					InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_SHIFT))
				{
					lastSelectedControl = selectedControlNode;
					//If controls was selected with SHIFT key pressed - we don't need mouseUp selection
					useMouseUpSelection = false;
				}
			}
			else
			{
				if (!InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_SHIFT))
					HierarchyTreeController::Instance()->ResetSelectedControl();
					
				HierarchyTreeController::Instance()->SelectControl(selectedControlNode);
			}
			inputState = InputStateSelection;
			SaveControlsPostion();
		}
		
		break;
	}
		
	inputPos = event->point;
    prevDragPoint = event->point;
    
    isStickedToX = false;
    isStickedToY = false;
    stickDelta.SetZero();

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
    
    HierarchyTreeScreenNode* screenNode = HierarchyTreeController::Instance()->GetActiveScreen();
    if (screenNode && inputState == InputStateGuideMove)
    {
        // Round() because we allow guide to be on the integer positions only.
        Vector2 localPoint = LocalToInternal(event->point);
        localPoint.x = Round(localPoint.x);
        localPoint.y = Round(localPoint.y);
        screenNode->MoveGuide(localPoint);
        return;
    }

	Vector2 delta = GetInputDelta(event->point).delta;
	
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
		if (InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_CTRL) && !copyControlsInProcess)
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

    HierarchyTreeScreenNode* screenNode = HierarchyTreeController::Instance()->GetActiveScreen();
    if (screenNode && inputState == InputStateGuideMove)
    {
        MoveGuide(screenNode);
        inputState = InputStateSelection;
        return;
    }

	if (inputState == InputStateDrag)
	{
        DefaultScreen::InputDelta inputDelta = GetInputDelta(event->point);
		ResetMoveDelta();
		MoveControl(inputDelta.delta, !(inputDelta.isStickedToX | inputDelta.isStickedToY));
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
			if (!InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_SHIFT))
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
			DoKeyboardMove(moveUp);
		}break;
		case DVKEY_DOWN:
		{
			DoKeyboardMove(moveDown);
		}break;
		case DVKEY_LEFT:
		{
			DoKeyboardMove(moveLeft);
		}break;
		case DVKEY_RIGHT:
		{
			DoKeyboardMove(moveRight);
		}break;
		case DVKEY_DELETE:
		case DVKEY_BACKSPACE:
		{
            HierarchyTreeScreenNode* screenNode = HierarchyTreeController::Instance()->GetActiveScreen();
            if (screenNode->AreGuidesEnabled() && screenNode->AreGuidesSelected())
            {
                DeleteSelectedGuides(screenNode);
            }
            else
            {
                DeleteSelectedControls();
            }
		}break;
		case DVKEY_C:
		{
			if (InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_CTRL))
			{
				CopyPasteController::Instance()->CopyControls(HierarchyTreeController::Instance()->GetActiveControlNodes());
			}
		}break;
		case DVKEY_V:
		{
			if (InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_CTRL))
			{
                const HierarchyTreeController::SELECTEDCONTROLNODES &selectedList = HierarchyTreeController::Instance()->GetActiveControlNodes();                
                if (selectedList.empty())
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

DefaultScreen::InputDelta DefaultScreen::GetInputDelta(const Vector2& point, bool applyScale)
{
    Vector2 delta = point - inputPos;
    HierarchyTreeScreenNode* screenNode = HierarchyTreeController::Instance()->GetActiveScreen();

    // Sticking to guides is supported  both for Drag and Size modes.
    if ((inputState == InputStateDrag || inputState == InputStateSize) && screenNode && screenNode->AreGuidesEnabled())
    {
        Vector2 alignOffset;
        int32 stickMode = CalculateStickToGuides(alignOffset);

        // To lock the appropriate sticks just need to know we are unlocked but
        // Guides require us to lock to some guide.
        Vector2 prevDragStep = point - prevDragPoint;
        if ((stickMode & StickedToX) && !isStickedToX)
        {
            // Locking X position.
            isStickedToX = true;
            stickDelta.x = delta.x - prevDragStep.x - (alignOffset.x * scale.x);
        }

        if ((stickMode & StickedToY) && !isStickedToY)
        {
            // Locking Y position.
            isStickedToY = true;
            stickDelta.y = delta.y - prevDragStep.y - (alignOffset.y * scale.y);
        }
        
        // To unlock the sticks have to check whether we moved above treshold.
        if (isStickedToX && fabs(stickDelta.x - delta.x) > GetGuideStickTreshold() * scale.x)
        {
            isStickedToX = false;
        }

        if (isStickedToY && fabs(stickDelta.y - delta.y) > GetGuideStickTreshold() * scale.y)
        {
            isStickedToY = false;
        }

        // Calculate the final delta, depending on what axes we are sticked to.
        delta.x = isStickedToX ? stickDelta.x : delta.x;
        delta.y = isStickedToY ? stickDelta.y : delta.y;
    }
    
    if (applyScale)
	{
		delta.x /= scale.x;
		delta.y /= scale.y;
	}

    prevDragPoint = point;
    
    return InputDelta(delta, isStickedToX, isStickedToY);
}

int32 DefaultScreen::CalculateStickToGuides(Vector2& offset) const
{
    if (inputState == InputStateDrag)
    {
        return CalculateStickToGuidesDrag(offset);
    }
    else if (inputState == InputStateSize)
    {
        return CalculateStickToGuidesSize(offset);
    }

    // Other input states aren't supported and this method should
    // not be called for them.
    DVASSERT(false);
    return 0;
}

int32 DefaultScreen::CalculateStickToGuidesDrag(Vector2& offset) const
{
    HierarchyTreeScreenNode* screenNode = HierarchyTreeController::Instance()->GetActiveScreen();
    if (!NeedCalculateStickMode(screenNode))
    {
        return NotSticked;
    }

    // Build the list of selected controls' rects and send it to the alignment.
    const HierarchyTreeController::SELECTEDCONTROLNODES &selectedList = HierarchyTreeController::Instance()->GetActiveControlNodes();
    List<Rect> controlRects;
    for (HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = selectedList.begin(); iter != selectedList.end(); ++iter)
    {
        HierarchyTreeControlNode* controlNode = (*iter);
        if (controlNode && controlNode->GetUIObject())
        {
            controlRects.push_back(controlNode->GetUIObject()->GetGeometricData().GetAABBox());
        }
    }
    
    return screenNode->CalculateStickToGuides(controlRects, offset);
}

int32 DefaultScreen::CalculateStickToGuidesSize(Vector2& offset) const
{
    HierarchyTreeScreenNode* screenNode = HierarchyTreeController::Instance()->GetActiveScreen();
    if (!NeedCalculateStickMode(screenNode))
    {
        return NotSticked;
    }
    
    // For Move mode we have to add the rect of the only side(s) of the control being moved.
    if (!lastSelectedControl || !lastSelectedControl->GetUIObject())
    {
        return NotSticked;
    }

    // Disable stick to centers for resize.
    int32 oldStickMode = screenNode->GetStickMode();
    screenNode->SetStickMode(StickToSides);

    // Check the "sticked to rect bounds" firstly.
    const Rect& selectedRect = lastSelectedControl->GetUIObject()->GetRect(true);
    int32 stickResult = CalculateStickToRectBounds(screenNode, selectedRect, offset);
    if (stickResult == NotSticked)
    {
        // Not sticked to rect bounds - check the center.
        stickResult = CalculateStickToRectCenter(screenNode, selectedRect, offset);
    }

    // Restore the stick mode.
    screenNode->SetStickMode(oldStickMode);

    return stickResult;
}

int32 DefaultScreen::CalculateStickToRectBounds(HierarchyTreeScreenNode* screenNode, const Rect& selectedRect, Vector2& offset) const
{
    List<Rect> controlRects;
    
    bool useStickedX = false;
    bool useStickedY = false;
    switch (resizeType)
    {
        case DAVA::ResizeTypeLeft:
        {
            controlRects.push_back(Rect(selectedRect.x, selectedRect.y, 0, 0));
            useStickedX = true;
            break;
        }
            
        case DAVA::ResizeTypeTop:
        {
            controlRects.push_back(Rect(selectedRect.x, selectedRect.y, 0, 0));
            useStickedY = true;
            break;
        }
            
        case DAVA::ResizeTypeRight:
        {
            controlRects.push_back(Rect(selectedRect.x + selectedRect.dx, selectedRect.y, 0, 0));
            useStickedX = true;
            break;
        }
            
        case DAVA::ResizeTypeBottom:
        {
            controlRects.push_back(Rect(selectedRect.x, selectedRect.y + selectedRect.dy, 0, 0));
            useStickedY = true;
            break;
        }
            
        case DAVA::ResizeTypeLeftTop:
        {
            controlRects.push_back(Rect(selectedRect.x, selectedRect.y, 0, 0));
            useStickedX = true;
            useStickedY = true;
            break;
        }
            
        case DAVA::ResizeTypeRigthTop:
        {
            controlRects.push_back(Rect(selectedRect.x + selectedRect.dx, selectedRect.y, 0, 0));
            useStickedX = true;
            useStickedY = true;
            break;
        }
            
        case DAVA::ResizeTypeLeftBottom:
        {
            controlRects.push_back(Rect(selectedRect.x, selectedRect.y + selectedRect.dy, 0, 0));
            useStickedX = true;
            useStickedY = true;
            break;
        }
            
        case DAVA::ResizeTypeRightBottom:
        {
            controlRects.push_back(Rect(selectedRect.x + selectedRect.dx, selectedRect.y + selectedRect.dy, 0, 0));
            useStickedX = true;
            useStickedY = true;
            break;
        }
            
        default:
        {
            break;
        }
    }

    int32 curStickResult = screenNode->CalculateStickToGuides(controlRects, offset);
    
    // Take only the stick modes allowed for this resize type.
    int32 finalStickResult = NotSticked;
    if (useStickedX && curStickResult & StickedToX)
    {
        finalStickResult |= StickedToX;
    }
    if (useStickedY && curStickResult & StickedToY)
    {
        finalStickResult |= StickedToY;
    }
    
    return finalStickResult;
}

int32 DefaultScreen::CalculateStickToRectCenter(HierarchyTreeScreenNode* screenNode, const Rect& selectedRect, Vector2& offset) const
{
    List<Rect> controlRects;
    controlRects.push_back(Rect(selectedRect.GetCenter().x, selectedRect.GetCenter().y, 0, 0));
    int32 stickResult = screenNode->CalculateStickToGuides(controlRects, offset);
    if (stickResult != NotSticked)
    {
        offset *= 2; //center point is changed twise slower than control size, so need to multiply.
    }
    
    return stickResult;
}

bool DefaultScreen::NeedCalculateStickMode(HierarchyTreeScreenNode* screenNode) const
{
    if (!screenNode)
    {
        return false;
    }
    
    const HierarchyTreeController::SELECTEDCONTROLNODES &selectedList = HierarchyTreeController::Instance()->GetActiveControlNodes();
    if (selectedList.empty())
    {
        return false;
    }
    
    return true;
}

int32 DefaultScreen::GetGuideStickTreshold() const
{
    HierarchyTreeScreenNode* screenNode = HierarchyTreeController::Instance()->GetActiveScreen();
    if (!screenNode)
    {
        return 0;
    }
    
    return screenNode->GetGuideStickTreshold();
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
			HierarchyTreeController::Instance()->SelectControl(newSelectedNode, HierarchyTreeController::DeferredExpand);
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

Rect DefaultScreen::GetControlRect(const HierarchyTreeControlNode* controlNode, bool checkAngle/*=false*/) const
{
	Rect rect;
	
	if (!controlNode)
		return rect;
	
	UIControl* control = controlNode->GetUIObject();
	if (!control)
		return rect;
	
    if(!checkAngle)
    {
        rect = control->GetRect(false);
    }
    else
    {
        rect = control->GetGeometricData().GetAABBox();
    }
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

int32 DefaultScreen::GetGuideMoveDelta()
{
	Qt::KeyboardModifiers modifiers = QApplication::queryKeyboardModifiers();
	return (modifiers & Qt::ShiftModifier) ? COARSE_GUIDE_MOVE_DELTA : FINE_GUIDE_MOVE_DELTA;
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
		Vector2 delta = GetInputDelta(pos, false).delta;
		ScreenWrapper::Instance()->RequestViewMove(-delta);
		inputPos = pos;
	}
}

bool DefaultScreen::IsControlVisible(const UIControl* uiControl) const
{
    return ( uiControl->GetVisible());
}

void DefaultScreen::SetScreenControl(ScreenControl* control)
{
    screenControl = control;
}

ScreenControl* DefaultScreen::GetScreenControl() const
{
    return screenControl;
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

void DefaultScreen::DrawGuides()
{
    HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
    if (!activeScreen || !activeScreen->AreGuidesEnabled())
    {
        return;
    }

    static const Color unselectedColor(0.0f, 0.8f, 1.0f, 1.0f);
    static const Color selectedColor(1.0f, 0.0f, 0.0f, 1.0f);

    Vector<float32> selectedGuides;
    Vector<float32> unselectedGuides;
    Rect rect = ScreenWrapper::Instance()->GetBackgroundFrameRect();
    
    const List<GuideData*>& guides = activeScreen->GetGuides(true);
    for (List<GuideData*>::const_iterator iter = guides.begin(); iter != guides.end(); iter ++)
    {
        GuideData* curData = *iter;
        Vector<float32>& curVector = curData->IsSelected() ? selectedGuides : unselectedGuides;
        switch (curData->GetType())
        {
            case GuideData::Horizontal:
            {
                curVector.push_back(rect.x);
                curVector.push_back(curData->GetPosition().y);
                curVector.push_back(rect.x + rect.dx);
                curVector.push_back(curData->GetPosition().y);
                break;
            }

            case GuideData::Vertical:
            {
                curVector.push_back(curData->GetPosition().x);
                curVector.push_back(rect.y);
                curVector.push_back(curData->GetPosition().x);
                curVector.push_back(rect.y + rect.dy);
                break;
            }
                
            case GuideData::Both:
            {
                curVector.push_back(rect.x);
                curVector.push_back(curData->GetPosition().y);
                curVector.push_back(rect.x + rect.dx);
                curVector.push_back(curData->GetPosition().y);
                curVector.push_back(curData->GetPosition().x);
                curVector.push_back(rect.y);
                curVector.push_back(curData->GetPosition().x);
                curVector.push_back(rect.y + rect.dy);
                break;
            }
                
            default:
            {
                break;
            }
        }
    }

    RenderSystem2D::Instance()->PushClip();
    RenderSystem2D::Instance()->SetClip(rect);
    Color oldColor = RenderManager::Instance()->GetColor();

    RenderManager::Instance()->SetColor(selectedColor);
    RenderHelper::Instance()->DrawLines(selectedGuides, RenderState::RENDERSTATE_2D_BLEND);
    RenderManager::Instance()->SetColor(unselectedColor);
    RenderHelper::Instance()->DrawLines(unselectedGuides, RenderState::RENDERSTATE_2D_BLEND);

    RenderManager::Instance()->SetColor(oldColor);
    RenderSystem2D::Instance()->PopClip();
}

// Screen scale/position is changed.
void DefaultScreen::SetScreenScaleChangedFlag()
{
    isNeedHandleScreenScaleChanged = true;
}

void DefaultScreen::SetScreenPositionChangedFlag()
{
    isNeedHandleScreenPositionChanged = true;
}

void DefaultScreen::HandleScreenScalePositionChanged()
{
    if (!(isNeedHandleScreenScaleChanged || isNeedHandleScreenPositionChanged))
    {
        return;
    }
    
    HierarchyTreeNode::HIERARCHYTREENODESLIST nodesList = HierarchyTreeController::Instance()->GetNodes();
    
    for (HierarchyTreeNode::HIERARCHYTREENODESITER iter = nodesList.begin(); iter != nodesList.end(); iter ++)
    {
        if (isNeedHandleScreenScaleChanged)
        {
            (*iter)->OnScreenScaleChanged();
        }
        
        if (isNeedHandleScreenPositionChanged)
        {
            (*iter)->OnScreenPositionChanged();
        }
    }
    
    isNeedHandleScreenScaleChanged = false;
    isNeedHandleScreenPositionChanged = false;
}
