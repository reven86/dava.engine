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


#include "Autotesting/TouchAction.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Autotesting/AutotestingSystem.h"

#include "Core/Core.h"

namespace DAVA
{

TouchAction::TouchAction(int32 _id) : Action()
    , id(_id)
{
    SetName("TouchAction");
}

TouchAction::~TouchAction()
{
}

void TouchAction::TouchDown(const Vector2 &point)
{
    //TODO: multitouch

    UIEvent touchDown;
    touchDown.phase = UIEvent::PHASE_BEGAN;
    touchDown.tid = id;
    touchDown.tapCount = 1;
    UIControlSystem::Instance()->RecalculatePointToPhysical(point, touchDown.physPoint);
    UIControlSystem::Instance()->RecalculatePointToVirtual(touchDown.physPoint, touchDown.point);
    //touchDown.physPoint = GetPhysicalPoint(point);
    //touchDown.point = GetVirtualPoint(touchDown.physPoint);

    ProcessInput(touchDown);
}

void TouchAction::TouchDown(const Vector<String> &controlPath, const Vector2 &offset)
{
    TouchDown(FindControlPosition(controlPath) + offset);
}

void TouchAction::TouchUp()
{
    UIEvent touchUp;
    if(!AutotestingSystem::Instance()->FindTouch(id, touchUp))
    {
        Logger::Error("TouchAction::TouchUp touch down not found");
    }
    touchUp.phase = UIEvent::PHASE_ENDED;
    touchUp.tid = id;

    ProcessInput(touchUp);
}

void TouchAction::TouchMove(const Vector2 &point)
{
    //Logger::Debug("TouchAction::TouchMove point=(%f, %f)", point.x, point.y);
    UIEvent touchMove;
    touchMove.tid = id;
    touchMove.tapCount = 1;
    UIControlSystem::Instance()->RecalculatePointToPhysical(point, touchMove.physPoint);
    UIControlSystem::Instance()->RecalculatePointToVirtual(touchMove.physPoint, touchMove.point);
    //touchMove.physPoint = GetPhysicalPoint(point);
    //touchMove.point = GetVirtualPoint(touchMove.physPoint);
        
    if(AutotestingSystem::Instance()->IsTouchDown(id))
    {
        touchMove.phase = UIEvent::PHASE_DRAG;
        ProcessInput(touchMove);
    }
    else
    {
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)        
        Logger::Warning("TouchAction::TouchMove point=(%f, %f) ignored no touch down found", point.x, point.y);
#else
        touchMove.phase = UIEvent::PHASE_MOVE;
        ProcessInput(touchMove);
#endif
    }
}


String TouchAction::Dump()
{
	String baseStr = Action::Dump();
	return Format("%s id=%d", baseStr.c_str(), id);
}

//----------------------------------------------------------------------

TouchDownAction::TouchDownAction(const Vector2 &_point, int32 _id) : TouchAction(_id)
    , point(_point)
{
    SetName("TouchDownAction");
}

TouchDownAction::~TouchDownAction()
{
}

void TouchDownAction::Execute()
{
    //Logger::Debug("TouchDownAction::Execute point=(%f, %f)",point.x, point.y);
    TouchDown(point);
    Action::Execute();
}

String TouchDownAction::Dump()
{
	String baseStr = TouchAction::Dump();
	return Format("%s point=(%.2f, %.2f)", baseStr.c_str(), point.x, point.y);
}

//----------------------------------------------------------------------
TouchDownControlAction::TouchDownControlAction(const String &_controlName, const Vector2 &offset, int32 _id) : TouchAction(_id)
	, touchOffset(offset)
{
    controlPath.push_back(_controlName);
}

TouchDownControlAction::TouchDownControlAction(const Vector<String> &_controlPath, const Vector2 &offset, int32 _id) : TouchAction(_id)
    , controlPath(_controlPath)
	, touchOffset(offset)
{
}

TouchDownControlAction::~TouchDownControlAction()
{
}

void TouchDownControlAction::Execute()
{
    TouchDown(controlPath, touchOffset);
    Action::Execute();
}

String TouchDownControlAction::Dump()
{
	String baseStr = TouchAction::Dump();
	String controlPathStr = PathToString(controlPath);
	return Format("%s controlPath=%s", baseStr.c_str(), controlPathStr.c_str());
}

//----------------------------------------------------------------------
TouchUpAction::TouchUpAction(int32 _id) : TouchAction(_id)
{
    SetName("TouchUpAction");
}
TouchUpAction::~TouchUpAction()
{
}

void TouchUpAction::Execute()
{
    //Logger::Debug("TouchUpAction::Execute");
    TouchUp();
    Action::Execute();
}
//----------------------------------------------------------------------

TouchMoveAction::TouchMoveAction(const Vector2 &_point, float32 _moveTime, int32 _id) : TouchAction(_id)
    , point(_point)
    , moveTime(_moveTime)
{
    SetName("TouchMoveAction");
}

TouchMoveAction::~TouchMoveAction()
{
}

void TouchMoveAction::Execute()
{
    Action::Execute();
}

String TouchMoveAction::Dump()
{
	String baseStr = TouchAction::Dump();
	return Format("%s point=(%.2f, %.2f) time=%.2f", baseStr.c_str(), point.x, point.y, moveTime);
}

void TouchMoveAction::Update(float32 timeElapsed)
{
    if(!isExecuted)
    {
        moveTime -= timeElapsed;

        if(!TestCondition())
        {
            Vector2 newPoint;
            UIEvent prevTouch;
            if(AutotestingSystem::Instance()->FindTouch(id, prevTouch))
            {
                //TODO: point or physPoint?
                newPoint = prevTouch.point + ((point - prevTouch.point)*timeElapsed/moveTime);
            }
            else
            {
                // mouse move
                Vector2 prevPoint = AutotestingSystem::Instance()->GetMousePosition();
                newPoint = prevPoint +  ((point - prevPoint)*timeElapsed/moveTime);
            }
            TouchMove(newPoint);
        }
        Action::Update(timeElapsed);
    }
}

bool TouchMoveAction::TestCondition()
{
    return (moveTime <= 0.0f);
}

//----------------------------------------------------------------------

TouchMoveDirAction::TouchMoveDirAction(const Vector2 &_direction, float32 _speed, float32 _moveTime, int32 _id) : TouchMoveAction(Vector2(), _moveTime, _id)
    , direction(_direction)
    , speed(_speed)
{
    SetName("TouchMoveDirAction");
}

void TouchMoveDirAction::Execute()
{
	point = AutotestingSystem::Instance()->GetMousePosition() + speed*moveTime*direction;
    TouchMoveAction::Execute();
}

String TouchMoveDirAction::Dump()
{
	String baseStr = TouchAction::Dump();
	return Format("%s direction=(%.2f, %.2f) time=%.2f speed=%.2f", baseStr.c_str(), direction.x, direction.y, moveTime, speed);
}

//----------------------------------------------------------------------

TouchMoveControlAction::TouchMoveControlAction(const String &_controlName, float32 _moveTime, const Vector2 &offset, int32 _id) 
	: TouchMoveAction(Vector2(), _moveTime, _id)
	, touchOffset(offset)
{
    SetName("TouchMoveControlAction");
    controlPath.push_back(_controlName);
}

TouchMoveControlAction::TouchMoveControlAction(const Vector<String> &_controlPath, float32 _moveTime, const Vector2 &offset, int32 _id) : TouchMoveAction(Vector2(), _moveTime, _id)
    , controlPath(_controlPath)
	, touchOffset(offset)
{

}

TouchMoveControlAction::~TouchMoveControlAction()
{
}

void TouchMoveControlAction::Execute()
{
    point = FindControlPosition(controlPath) + touchOffset;
    TouchMoveAction::Execute();
}

String TouchMoveControlAction::Dump()
{
	String baseStr = TouchMoveAction::Dump();
	String controlPathStr = PathToString(controlPath);
	return Format("%s controlPath=%s", baseStr.c_str(), controlPathStr.c_str());
}

//----------------------------------------------------------------------
ScrollControlAction::ScrollControlAction(const String &_controlName, int32 _id, float32 timeout, const Vector2 &offset) : WaitAction(timeout)
    , id(_id)
    , currentAction(NULL)
	, isFound(false)
	, wasFound(false)
	, touchOffset(offset)
{
    SetName("ScrollControlAction");
    controlPath.push_back(_controlName);
}
ScrollControlAction::ScrollControlAction(const Vector<String> &_controlPath, int32 _id, float32 timeout, const Vector2 &offset) : WaitAction(timeout)
    , controlPath(_controlPath)
    , id(_id)
    , currentAction(NULL)
    , isFound(false)
	, wasFound(false)
	, touchOffset(offset)
{
    SetName("ScrollControlAction");
}

ScrollControlAction::~ScrollControlAction()
{
    for(Deque<Action*>::iterator it = actions.begin(); it != actions.end(); ++it)
    {
        SafeRelease(*it);
    }
    actions.clear();
}

void ScrollControlAction::Update(float32 timeElapsed)
{
    WaitAction::Update(timeElapsed);
    if(!isExecuted)
    {
        if(actions.empty())
        {
			if(!wasFound)
            {
                //add: touch down, touch move, touch up
                actions.push_back(new TouchDownAction(touchDownPoint + touchOffset, id));
                actions.push_back(new TouchMoveAction(touchUpPoint + touchOffset, 0.2f, id));
                actions.push_back(new TouchUpAction(id));
            }
			else
			{
				isFound = wasFound;
			}
			wasFound = (FindControl(controlPath) != NULL);
        }
        else
        {
            // update touch move
            if(!currentAction)
            {
                currentAction = actions.front();
                if(currentAction)
                {
                    currentAction->Execute();
                }
            }

            if(currentAction)
            {
                if(currentAction->IsExecuted())
                {
                    SafeRelease(currentAction);
                    actions.pop_front();
                }
                else
                {
                    currentAction->Update(timeElapsed);
                }
            }
            else
            {
                actions.pop_front();
            }
        }
    }
}

void ScrollControlAction::Execute()
{
    FindScrollPoints();
    WaitAction::Execute();
}

String ScrollControlAction::Dump()
{
	String baseStr = WaitAction::Dump();
	String controlPathStr = PathToString(controlPath);
	return Format("%s controlPath=%s", baseStr.c_str(), controlPathStr.c_str());
}

bool ScrollControlAction::TestCondition()
{
    if(WaitAction::TestCondition())
    {
        AutotestingSystem::Instance()->OnError(Format("ScrollControlAction %s timeout", PathToString(controlPath).c_str()));
        return true;
    }
    return isFound;
    //return (FindControl(controlPath) != NULL);
}

void ScrollControlAction::FindScrollPoints()
{
    if(controlPath.empty()) return;

    Vector<String> parentPath;
    for(uint32 i = 0; i < controlPath.size() - 1; ++i)
    {
        parentPath.push_back(controlPath[i]);
    }
    String indexStr = controlPath.back();

	bool isNumber = true;
    int32 index = atoi(indexStr.c_str());
    if((index == 0) && (Format("%d",index) != indexStr))
    {
		// not number
		isNumber = false;
	}

    UIControl* parent = NULL;
    if(parentPath.empty())
    {
        // parent is screen
        AutotestingSystem::Instance()->OnError(Format("ScrollControlAction %s unable to scroll screen", PathToString(controlPath).c_str()));
    }
    else
    {
        parent = FindControl(parentPath);
        // parent is something inside screen
        UIList* parentList = dynamic_cast<UIList*>(parent);
        if(parentList)
        {
            // parent is list
            Rect parentRect(parentList->GetGeometricData().GetUnrotatedRect());

            //TODO: find min and max visible index
            int32 minVisibleIndex = 10000;
            int32 maxVisibleIndex = 0;
            const List<UIControl*> &cells = parentList->GetVisibleCells();
            for(List<UIControl*>::const_iterator it = cells.begin(); it != cells.end(); ++it)
            {
                UIListCell* cell = dynamic_cast<UIListCell*>(*it);
                if(cell && IsCenterInside(parentList, cell))
                {
                    int32 cellIndex = cell->GetIndex();
                    if(cellIndex < minVisibleIndex)
                    {
                        minVisibleIndex = cellIndex;
                    }
                    
                    if(maxVisibleIndex < cellIndex)
                    {
                        maxVisibleIndex = cellIndex;
                    }
                }
            }

            if(parentList->GetOrientation() == UIList::ORIENTATION_HORIZONTAL)
            {
				if(isNumber)
				{
					// horizontal
					if(maxVisibleIndex < index)
					{
						// touch up < down  <-
						touchDownPoint = Vector2(parentRect.x + 0.55f*parentRect.dx, parentRect.y + 0.5f*parentRect.dy);
						touchUpPoint = Vector2(parentRect.x + 0.45f*parentRect.dx, parentRect.y + 0.5f*parentRect.dy);
					}
					else
					{
						// touch down < up ->
						touchDownPoint = Vector2(parentRect.x + 0.45f*parentRect.dx, parentRect.y + 0.5f*parentRect.dy);
						touchUpPoint = Vector2(parentRect.x + 0.55f*parentRect.dx, parentRect.y + 0.5f*parentRect.dy);
					}
				}
				else
				{
					//TODO: smart search - first to the beginning of the list, then to the end
					
					// touch up < down  <-
					touchDownPoint = Vector2(parentRect.x + 0.55f*parentRect.dx, parentRect.y + 0.5f*parentRect.dy);
					touchUpPoint = Vector2(parentRect.x + 0.45f*parentRect.dx, parentRect.y + 0.5f*parentRect.dy);
				}
            }
            else
            {
				if(isNumber)
				{
					// vertical
					if(maxVisibleIndex < index)
					{
						// touch up < down  ^
						touchDownPoint = Vector2(parentRect.x + 0.5f*parentRect.dx, parentRect.y + 0.55f*parentRect.dy);
						touchUpPoint = Vector2(parentRect.x + 0.5f*parentRect.dx, parentRect.y + 0.45f*parentRect.dy);
					}
					else
					{
						// touch down < up
						touchDownPoint = Vector2(parentRect.x + 0.5f*parentRect.dx, parentRect.y + 0.45f*parentRect.dy);
						touchUpPoint = Vector2(parentRect.x + 0.5f*parentRect.dx, parentRect.y + 0.55f*parentRect.dy);
					}
				}
				else
				{
					//TODO: smart search -  first to the beginning of the list, then to the end
					
					// touch up < down  ^
					touchDownPoint = Vector2(parentRect.x + 0.5f*parentRect.dx, parentRect.y + 0.55f*parentRect.dy);
					touchUpPoint = Vector2(parentRect.x + 0.5f*parentRect.dx, parentRect.y + 0.45f*parentRect.dy);
				}
                
            }
            Logger::Debug("ScrollControlAction::FindScrollPoints %s scroll from (%f, %f) to (%f, %f)", PathToString(controlPath).c_str(), touchDownPoint.x, touchDownPoint.y, touchUpPoint.x, touchUpPoint.y);
        }
        else
        {
            // parent is not list
            //TODO: scroll controls
            AutotestingSystem::Instance()->OnError(Format("ScrollControlAction %s unable to scroll control", PathToString(controlPath).c_str()));
        }
    }
}

MultitouchAction::MultitouchAction() : Action()
{
    SetName("MultitouchAction");
}

MultitouchAction::~MultitouchAction()
{
	for_each(touchActions.begin(), touchActions.end(), SafeRelease<TouchAction>);
	touchActions.clear();
}

void MultitouchAction::AddTouch(TouchAction *touchAction)
{
	if(touchAction)
	{
		Logger::Debug("MultitouchAction::AddTouch %s", touchAction->GetName().c_str());
		touchAction->Retain();
		touchActions.push_back(touchAction);
	}
}

void MultitouchAction::Update(float32 timeElapsed)
{
	int32 touchActionsCount = touchActions.size();
	for(int32 i = 0; i < touchActionsCount; ++i)
	{
		if(!touchActions[i]->IsExecuted())
		{
			touchActions[i]->Update(timeElapsed);
		}
	}
	Action::Update(timeElapsed);
}

void MultitouchAction::Execute()
{
	int32 touchActionsCount = touchActions.size();
	for(int32 i = 0; i < touchActionsCount; ++i)
	{
		touchActions[i]->Execute();
	}
	Action::Execute();
}

String MultitouchAction::Dump()
{
	return Action::Dump(); //TODO: get detailed info
}

bool MultitouchAction::TestCondition()
{
	int32 touchActionsCount = touchActions.size();
	for(int32 i = 0; i < touchActionsCount; ++i)
	{
		if(!touchActions[i]->IsExecuted())
		{
			return false;
		}
	}
	return true;
}

}; // namespace DAVA

#endif //__DAVAENGINE_AUTOTESTING__