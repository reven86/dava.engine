#include "UIInputSystem.h"
#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "Debug/Replay.h"

namespace DAVA
{

UIInputSystem::UIInputSystem()
{
    exclusiveInputLocker = NULL;
    lockInputCounter = 0;
}

UIInputSystem::~UIInputSystem()
{

}

uint32 UIInputSystem::GetRequiredComponents() const
{
    return 0;
}

uint32 UIInputSystem::GetType() const
{
    return UISystem::UI_INPUT_SYSTEM;
}

void UIInputSystem::Process()
{
    
}


bool UIInputSystem::SystemProcessInput(UIControl* control, UIEvent *currentInput)
{
    if (!control->inputEnabled || !control->GetSystemVisible() || control->controlState & UIControl::STATE_DISABLED)
    {
        return false;
    }
    if (GetExclusiveInputLocker() && GetExclusiveInputLocker() != control)
    {
        return false;
    }

    switch (currentInput->phase)
    {
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
    case UIEvent::PHASE_KEYCHAR:
    {
        if (control->customInput != (int)NULL)
        {
            control->customInput(currentInput);
        }
    }
    break;
    case UIEvent::PHASE_MOVE:
    {
        if (!currentInput->touchLocker && control->IsPointInside(currentInput->point))
        {
            UIControlSystem::Instance()->SetHoveredControl(control);
            if (control->customInput != (int)NULL)
            {
                control->customInput(currentInput);
            }
            return true;
        }
    }
    break;
    case UIEvent::PHASE_WHEEL:
    {
        if (control->customInput != (int)NULL)
        {
            control->customInput(currentInput);
        }
    }
    break;
#endif
    case UIEvent::PHASE_BEGAN:
    {
        if (!currentInput->touchLocker && control->IsPointInside(currentInput->point))
        {
            if (control->multiInput || !control->currentInputID)
            {

                control->controlState |= UIControl::STATE_PRESSED_INSIDE;
                control->controlState &= ~UIControl::STATE_NORMAL;
                ++control->touchesInside;
                ++control->totalTouches;
                currentInput->controlState = UIEvent::CONTROL_STATE_INSIDE;

                // Yuri Coder, 2013/12/18. Set the touch lockers before the EVENT_TOUCH_DOWN handler
                // to have possibility disable control inside the EVENT_TOUCH_DOWN. See also DF-2943.
                currentInput->touchLocker = control;
                if (control->exclusiveInput)
                {
                    SetExclusiveInputLocker(control, currentInput->tid);
                }

                control->PerformEventWithData(UIControl::EVENT_TOUCH_DOWN, currentInput);

                if (!control->multiInput)
                {
                    control->currentInputID = currentInput->tid;
                }

                if (control->customInput != (int)NULL)
                {
                    control->customInput(currentInput);
                }
                return true;
            }
            else
            {
                currentInput->touchLocker = control;
                return true;
            }

        }
    }
    break;
    case UIEvent::PHASE_DRAG:
    {
        if (currentInput->touchLocker == control)
        {
            if (control->multiInput || control->currentInputID == currentInput->tid)
            {
                if (control->controlState & UIControl::STATE_PRESSED_INSIDE || control->controlState & UIControl::STATE_PRESSED_OUTSIDE)
                {
                    if (control->IsPointInside(currentInput->point, true))
                    {
                        if (currentInput->controlState == UIEvent::CONTROL_STATE_OUTSIDE)
                        {
                            currentInput->controlState = UIEvent::CONTROL_STATE_INSIDE;
                            ++control->touchesInside;
                            if (control->touchesInside > 0)
                            {
                                control->controlState |= UIControl::STATE_PRESSED_INSIDE;
                                control->controlState &= ~UIControl::STATE_PRESSED_OUTSIDE;
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
                                control->controlState |= UIControl::STATE_HOVER;
#endif
                            }
                        }
                    }
                    else
                    {
                        if (currentInput->controlState == UIEvent::CONTROL_STATE_INSIDE)
                        {
                            currentInput->controlState = UIEvent::CONTROL_STATE_OUTSIDE;
                            --control->touchesInside;
                            if (control->touchesInside == 0)
                            {
                                control->controlState |= UIControl::STATE_PRESSED_OUTSIDE;
                                control->controlState &= ~UIControl::STATE_PRESSED_INSIDE;
                            }
                        }
                    }
                }

                if (control->customInput != (int)NULL)
                {
                    control->customInput(currentInput);
                }
            }
            return true;
        }
    }
    break;
    case UIEvent::PHASE_ENDED:
    {
        if (currentInput->touchLocker == control)
        {
            if (control->multiInput || control->currentInputID == currentInput->tid)
            {
                if (control->customInput != (int)NULL)
                {
                    control->customInput(currentInput);
                }
                if (currentInput->tid == control->currentInputID)
                {
                    control->currentInputID = 0;
                }
                if (control->totalTouches > 0)
                {
                    --control->totalTouches;
                    if (currentInput->controlState == UIEvent::CONTROL_STATE_INSIDE)
                    {
                        --control->touchesInside;
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
                        if (control->totalTouches == 0)
                        {
                            control->controlState |= UIControl::STATE_HOVER;
                        }
#endif
                    }

                    currentInput->controlState = UIEvent::CONTROL_STATE_RELEASED;

                    if (control->totalTouches == 0)
                    {
                        if (control->IsPointInside(currentInput->point, true))
                        {
                            if (UIControlSystem::Instance()->GetFocusedControl() != control && control->focusEnabled)
                            {
                                UIControlSystem::Instance()->SetFocusedControl(control, false);
                            }
                            control->PerformEventWithData(UIControl::EVENT_TOUCH_UP_INSIDE, currentInput);
                        }
                        else
                        {
                            control->PerformEventWithData(UIControl::EVENT_TOUCH_UP_OUTSIDE, currentInput);
                        }
                        control->controlState &= ~UIControl::STATE_PRESSED_INSIDE;
                        control->controlState &= ~UIControl::STATE_PRESSED_OUTSIDE;
                        control->controlState |= UIControl::STATE_NORMAL;
                        if (GetExclusiveInputLocker() == control)
                        {
                            SetExclusiveInputLocker(NULL, -1);
                        }
                    }
                    else if (control->touchesInside <= 0)
                    {
                        control->controlState |= UIControl::STATE_PRESSED_OUTSIDE;
                        control->controlState &= ~UIControl::STATE_PRESSED_INSIDE;
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
                        control->controlState &= ~UIControl::STATE_HOVER;
#endif
                    }
                }
            }
            currentInput->touchLocker = NULL;
            return true;
        }
    }
    break;
    case UIEvent::PHASE_JOYSTICK:
    {
        if (control->customInput != (int)NULL)
        {
            control->customInput(currentInput);
        }
    }
    }

    return false;
}

bool UIInputSystem::SystemInput(UIControl* control, UIEvent *currentInput)
{
    inputCounter++;
    control->isUpdated = true;

    if (!control->GetSystemVisible())
        return false;

    //if(currentInput->touchLocker != this)
    {
        if (control->clipContents
            && (currentInput->phase != UIEvent::PHASE_DRAG
            && currentInput->phase != UIEvent::PHASE_ENDED
            && currentInput->phase != UIEvent::PHASE_KEYCHAR
            && currentInput->phase != UIEvent::PHASE_JOYSTICK))
        {
            if (!control->IsPointInside(currentInput->point))
            {
                return false;
            }
        }

        List<UIControl*>::reverse_iterator it = control->childs.rbegin();
        List<UIControl*>::reverse_iterator itEnd = control->childs.rend();
        for (; it != itEnd; ++it)
        {
            (*it)->isUpdated = false;
        }

        it = control->childs.rbegin();
        itEnd = control->childs.rend();
        while (it != itEnd)
        {
            control->isIteratorCorrupted = false;
            UIControl *current = *it;
            if (!current->isUpdated)
            {
                current->Retain();
                if (current->inputProcessorsCount > 0 && (current->customSystemInput != (int)0 ? current->customSystemInput(currentInput) : SystemInput(current, currentInput)) )
                {
                    current->Release();
                    return true;
                }
                current->Release();
                if (control->isIteratorCorrupted)
                {
                    it = control->childs.rbegin();
                    continue;
                }
            }
            ++it;
        }
    }
    if (control->customSystemProcessInput != (int)0)
    {
        return control->customSystemProcessInput(currentInput);
    }
    return SystemProcessInput(control, currentInput);
}

void UIInputSystem::SystemInputCancelled(UIControl* control, UIEvent *currentInput)
{
    if (currentInput->controlState != UIEvent::CONTROL_STATE_RELEASED)
    {
        --control->totalTouches;
    }
    if (currentInput->controlState == UIEvent::CONTROL_STATE_INSIDE)
    {
        --control->touchesInside;
    }

    if (control->touchesInside == 0)
    {
        control->controlState &= ~UIControl::STATE_PRESSED_INSIDE;
        control->controlState &= ~UIControl::STATE_PRESSED_OUTSIDE;
        control->controlState |= UIControl::STATE_NORMAL;
        if (GetExclusiveInputLocker() == control)
        {
            SetExclusiveInputLocker(NULL, -1);
        }
    }

    currentInput->controlState = UIEvent::CONTROL_STATE_RELEASED;
    if (currentInput->tid == control->currentInputID)
    {
        control->currentInputID = 0;
    }
    currentInput->touchLocker = NULL;

    if (control->customInputCancelled != (int)NULL)
    {
        control->customInputCancelled(currentInput);
    }
}

void UIInputSystem::SwitchInputToControl(int32 eventID, UIControl *targetControl)
{
	for (Vector<UIEvent>::iterator it = totalInputs.begin(); it != totalInputs.end(); it++) 
	{
		if((*it).tid == eventID)
		{
			CancelInput(&(*it));
			
			if(targetControl->IsPointInside((*it).point))
			{
				(*it).controlState = UIEvent::CONTROL_STATE_INSIDE;
				targetControl->touchesInside++;
			}
			else 
			{
				(*it).controlState = UIEvent::CONTROL_STATE_OUTSIDE;
			}
			(*it).touchLocker = targetControl;
			targetControl->currentInputID = eventID;
			if(targetControl->GetExclusiveInput())
			{
				SetExclusiveInputLocker(targetControl, eventID);
			}
			else 
			{
				SetExclusiveInputLocker(NULL, -1);
			}

			targetControl->totalTouches++;
		}
	}
}

void UIInputSystem::OnInput(int32 touchType, const Vector<UIEvent> &activeInputs, const Vector<UIEvent> &allInputs, bool fromReplay/* = false*/)
{
    inputCounter = 0;
	if(Replay::IsPlayback() && !fromReplay) return;
	if (lockInputCounter > 0)return;

	if(UIControlSystem::Instance()->frameSkip <= 0)
	{
		if(Replay::IsRecord())
		{
			int32 count = (int32)activeInputs.size();
			Replay::Instance()->RecordEventsCount(count);
			for(Vector<UIEvent>::const_iterator it = activeInputs.begin(); it != activeInputs.end(); ++it) 
			{
				UIEvent ev = *it;
                ev.point = VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual(ev.physPoint);
				Replay::Instance()->RecordEvent(&ev);
			}

			count = (int32)allInputs.size();
			Replay::Instance()->RecordEventsCount(count);
			for(Vector<UIEvent>::const_iterator it = allInputs.begin(); it != allInputs.end(); ++it) 
			{
				UIEvent ev = *it;
                ev.point = VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual(ev.physPoint);
				Replay::Instance()->RecordEvent(&ev);
			}
		}

		//check all touches for active state
		for (Vector<UIEvent>::iterator it = totalInputs.begin(); it != totalInputs.end(); it++) 
		{
			(*it).activeState = UIEvent::ACTIVITY_STATE_INACTIVE;
			
			for (Vector<UIEvent>::const_iterator wit = activeInputs.begin(); wit != activeInputs.end(); wit++) 
			{
				if((*it).tid == (*wit).tid)
				{
					if((*it).phase == (*wit).phase && (*it).physPoint == (*wit).physPoint)
					{
						(*it).activeState = UIEvent::ACTIVITY_STATE_ACTIVE;
					}
					else 
					{
						(*it).activeState = UIEvent::ACTIVITY_STATE_CHANGED;
					}
					
					(*it).phase = (*wit).phase;
					(*it).timestamp = (*wit).timestamp;
					(*it).physPoint = (*wit).physPoint;
                    (*it).point = VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual((*it).physPoint);
					(*it).tapCount = (*wit).tapCount;
					(*it).inputHandledType = (*wit).inputHandledType;
					break;
				}
			}
			if((*it).activeState == UIEvent::ACTIVITY_STATE_INACTIVE)
			{
				for (Vector<UIEvent>::const_iterator wit = allInputs.begin(); wit != allInputs.end(); wit++) 
				{
					if((*it).tid == (*wit).tid)
					{
						if((*it).phase == (*wit).phase && (*it).point == (*wit).point)
						{
							(*it).activeState = UIEvent::ACTIVITY_STATE_ACTIVE;
						}
						else 
						{
							(*it).activeState = UIEvent::ACTIVITY_STATE_CHANGED;
						}
						
						(*it).phase = (*wit).phase;
						(*it).timestamp = (*wit).timestamp;
						(*it).physPoint = (*wit).physPoint;
						(*it).point = (*wit).point;
                        (*it).point = VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual((*it).physPoint);
						(*it).tapCount = (*wit).tapCount;
						(*it).inputHandledType = (*wit).inputHandledType;
						break;
					}
				}
			}
		}
		
		//add new touches
		for (Vector<UIEvent>::const_iterator wit = activeInputs.begin(); wit != activeInputs.end(); wit++) 
		{
			bool isFind = FALSE;
			for (Vector<UIEvent>::iterator it = totalInputs.begin(); it != totalInputs.end(); it++) 
			{
				if((*it).tid == (*wit).tid)
				{
					isFind = TRUE;
                    break;
				}
			}
			if(!isFind)
			{
				totalInputs.push_back((*wit));
                
                Vector<UIEvent>::reference curr(totalInputs.back());
				curr.activeState = UIEvent::ACTIVITY_STATE_CHANGED;
                //curr.phase = UIEvent::PHASE_BEGAN;
                curr.point = VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual(curr.physPoint);
			}
		}
		for (Vector<UIEvent>::const_iterator wit = allInputs.begin(); wit != allInputs.end(); wit++) 
		{
			bool isFind = FALSE;
			for (Vector<UIEvent>::iterator it = totalInputs.begin(); it != totalInputs.end(); it++) 
			{
				if((*it).tid == (*wit).tid)
				{
					isFind = TRUE;
                    break;
				}
			}
			if(!isFind)
			{
				totalInputs.push_back((*wit));
                
                Vector<UIEvent>::reference curr(totalInputs.back());
				curr.activeState = UIEvent::ACTIVITY_STATE_CHANGED;
                curr.point = VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual(curr.physPoint);
			}
		}
		
		//removes inactive touches and canceled touches
		for (Vector<UIEvent>::iterator it = totalInputs.begin(); it != totalInputs.end();)
		{
			if((*it).activeState == UIEvent::ACTIVITY_STATE_INACTIVE || (*it).phase == UIEvent::PHASE_CANCELLED)
			{
                if ((*it).phase != UIEvent::PHASE_ENDED)
                {
                    CancelInput(&(*it));
                }
				totalInputs.erase(it);
				it = totalInputs.begin();
				if(it == totalInputs.end())
				{
					break;
				}
                continue;
			}
            it++;
		}
		
        UIControl* currentScreen = UIControlSystem::Instance()->GetScreen();
		if(currentScreen)
		{
			for(Vector<UIEvent>::iterator it = totalInputs.begin(); it != totalInputs.end(); it++) 
			{
				if((*it).activeState == UIEvent::ACTIVITY_STATE_CHANGED)
				{
                    UIControl* popupContainer = UIControlSystem::Instance()->GetPopupContainer();
					if(!SystemInput(popupContainer, &(*it)))
					{
						SystemInput(currentScreen, &(*it));
					}
				}
				if(totalInputs.empty())
				{
					break;
				}
			}
		}
	}
}

void UIInputSystem::OnInput(UIEvent * event)
{
    UIControl* currentScreen = UIControlSystem::Instance()->GetScreen();
	if(currentScreen)
	{
        UIControl* popupContainer = UIControlSystem::Instance()->GetPopupContainer();
		if(!SystemInput(popupContainer, event))
		{
			SystemInput(currentScreen, event);
		}
	}
}

void UIInputSystem::CancelInput(UIEvent *touch)
{
	if(touch->touchLocker)
	{
        if (touch->touchLocker->customSystemInputCancelled != NULL)
        {
            touch->touchLocker->customSystemInputCancelled(touch);
        }
        else
        {
            SystemInputCancelled(touch->touchLocker, touch);
        }
	}
    UIControl* currentScreen = UIControlSystem::Instance()->GetScreen();
	if (touch->touchLocker != currentScreen)
	{
        if (currentScreen->customSystemInputCancelled != NULL)
        {
            currentScreen->customSystemInputCancelled(touch);
        }
        else
        {
            SystemInputCancelled(currentScreen, touch);
        }
	}
}

void UIInputSystem::CancelAllInputs()
{
	for (Vector<UIEvent>::iterator it = totalInputs.begin(); it != totalInputs.end(); it++) 
	{
		CancelInput(&(*it));
	}
	totalInputs.clear();
}

void UIInputSystem::CancelInputs(UIControl *control, bool hierarchical)
{
	for (Vector<UIEvent>::iterator it = totalInputs.begin(); it != totalInputs.end(); it++) 
	{
        if (!hierarchical)
        {
            if (it->touchLocker == control)
            {
                CancelInput(&(*it));
                break;
            }
            continue;
        }
        UIControl * parentLockerControl = it->touchLocker;
        while(parentLockerControl)
        {
            if(control == parentLockerControl)
            {
                CancelInput(&(*it));
                break;
            }
            parentLockerControl = parentLockerControl->GetParent();
        }
	}
}

int32 UIInputSystem::LockInput()
{
	lockInputCounter++;
	if (lockInputCounter > 0)
	{
		CancelAllInputs();
	}
	return lockInputCounter;
}

int32 UIInputSystem::UnlockInput()
{
	DVASSERT(lockInputCounter != 0);

	lockInputCounter--;
	if (lockInputCounter == 0)
	{
		// VB: Done that because hottych asked to do that.
		CancelAllInputs();
	}
	return lockInputCounter;
}
	
int32 UIInputSystem::GetLockInputCounter() const
{
	return lockInputCounter;
}

const Vector<UIEvent> & UIInputSystem::GetAllInputs()
{
	return totalInputs;
}
	
void UIInputSystem::SetExclusiveInputLocker(UIControl *locker, int32 lockEventId)
{
	SafeRelease(exclusiveInputLocker);
    if (locker != NULL)
    {
        for (Vector<UIEvent>::iterator it = totalInputs.begin(); it != totalInputs.end(); it++)
        {
            if (it->tid != lockEventId && it->touchLocker != locker)
            {//cancel all inputs excepts current input and inputs what allready handles by this locker.
                CancelInput(&(*it));
            }
        }
    }
	exclusiveInputLocker = SafeRetain(locker);
}
	
UIControl *UIInputSystem::GetExclusiveInputLocker()
{
	return exclusiveInputLocker;
}

}


