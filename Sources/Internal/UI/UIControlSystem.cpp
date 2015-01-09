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



#include "UI/UIControlSystem.h"
#include "UI/UIScreen.h"
#include "FileSystem/Logger.h"
#include "Render/RenderManager.h"
#include "Render/OcclusionQuery.h"
#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"
#include "Debug/Replay.h"
#include "Debug/Stats.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#include "UI/Systems/UIRenderSystem.h"
#include "UI/Systems/UIUpdateSystem.h"
#include "UI/Systems/UIInputSystem.h"

namespace DAVA 
{

const FastName FRAME_QUERY_UI_DRAW("OcclusionStatsUIDraw");

UIControlSystem::UIControlSystem()
    : systems(UISystem::UI_CONTROL_SYSTEMS_COUNT, NULL)
{
    systems[UISystem::UI_RENDER_SYSTEM] = new UIRenderSystem();
	systems[UISystem::UI_UPDATE_SYSTEM] = new UIUpdateSystem();
    systems[UISystem::UI_INPUT_SYSTEM] = new UIInputSystem();

	screenLockCount = 0;
	frameSkip = 0;
	transitionType = 0;
	
	nextScreenTransition = 0;
	currentScreen = 0;
	nextScreen = 0;
	prevScreen = NULL;
	removeCurrentScreen = false;
    hovered = NULL;
    focusedControl = NULL;
	//mainControl = 0;

	popupContainer = new UIControl(Rect(0, 0, 1, 1));
	popupContainer->SetInputEnabled(false);
	
	baseGeometricData.position = Vector2(0, 0);
	baseGeometricData.size = Vector2(0, 0);
	baseGeometricData.pivotPoint = Vector2(0, 0);
	baseGeometricData.scale = Vector2(1.0f, 1.0f);
	baseGeometricData.angle = 0;

    ui3DViewCount = 0;
}

UIControlSystem::~UIControlSystem()
{
    SafeRelease(currentScreen);
    SafeRelease(popupContainer);

    uint32 size = (uint32)systems.size();
    for (uint32 i = 0; i < size; ++i)
    {
        SafeDelete(systems[i]);
    }
    systems.clear();
}
	
void UIControlSystem::SetScreen(UIScreen *_nextScreen, UIScreenTransition * _transition)
{
	if (_nextScreen == currentScreen)
	{
		if (nextScreen != 0)
		{
			SafeRelease(nextScreenTransition);
			SafeRelease(nextScreen);
		}
		return;
	}

	if (nextScreen)
	{
		Logger::Warning("2 screen switches during one frame.");
	}
    
	// 2 switches on one frame can cause memory leak
	SafeRelease(nextScreenTransition);
	SafeRelease(nextScreen);
    
	nextScreenTransition = SafeRetain(_transition);
	
	if (_nextScreen == 0)
	{
		removeCurrentScreen = true;
	}
	
	nextScreen = SafeRetain(_nextScreen);
}
	
	
void UIControlSystem::ReplaceScreen(UIScreen *newMainControl)
{
	prevScreen = currentScreen;
	currentScreen = newMainControl;
    NotifyListenersDidSwitch(currentScreen);
}

	
UIScreen *UIControlSystem::GetScreen()
{
	return currentScreen;	
}
	
void UIControlSystem::AddPopup(UIPopup *newPopup)
{
    Set<UIPopup*>::const_iterator it = popupsToRemove.find(newPopup);
    if (popupsToRemove.end() != it)
    {
        popupsToRemove.erase(it);
        return;
    }

    newPopup->LoadGroup();
    popupContainer->AddControl(newPopup);
}
	
void UIControlSystem::RemovePopup(UIPopup *popup)
{
    if (popupsToRemove.count(popup))
    {
        Logger::Warning("[UIControlSystem::RemovePopup] attempt to double remove popup during one frame.");
        return;
    }

    const List<UIControl*> &popups = popupContainer->GetChildren();
    if (popups.end() == std::find(popups.begin(), popups.end(), DynamicTypeCheck<UIPopup*>(popup)))
    {
        Logger::Error("[UIControlSystem::RemovePopup] attempt to remove uknown popup.");
        DVASSERT(false);
        return;
    }

    popupsToRemove.insert(popup);
}
	
void UIControlSystem::RemoveAllPopups()
{
    popupsToRemove.clear();
	const List<UIControl*> &totalChilds = popupContainer->GetChildren();
	for (List<UIControl*>::const_iterator it = totalChilds.begin(); it != totalChilds.end(); it++)
	{
		popupsToRemove.insert(DynamicTypeCheck<UIPopup*>(*it));
	}
}
	
UIControl *UIControlSystem::GetPopupContainer()
{
	return popupContainer;
}

	
void UIControlSystem::Reset()
{
	SetScreen(0);
}
	
void UIControlSystem::ProcessScreenLogic()
{
	/*
	 if next screen or we need to remove current screen
	 */
	if (screenLockCount == 0 && (nextScreen || removeCurrentScreen))
	{
        UIScreen* nextScreenProcessed = 0;
        UIScreenTransition* transitionProcessed = 0;
        
        nextScreenProcessed = nextScreen;
        transitionProcessed = nextScreenTransition;
        nextScreen = 0; // functions called by this method can request another screen switch (for example, LoadResources)
        nextScreenTransition = 0;
        
        GetSystem<UIInputSystem>()->LockInput();
		GetSystem<UIInputSystem>()->CancelAllInputs();
		
        NotifyListenersWillSwitch(nextScreenProcessed);

		// If we have transition set
		if (transitionProcessed)
		{
			LockSwitch();

			// check if we have not loading transition
			if (!transitionProcessed->IsLoadingTransition())
			{
				// start transition and set currentScreen 
				transitionProcessed->StartTransition(currentScreen, nextScreenProcessed);
				currentScreen = transitionProcessed;
			}else
			{
				// if we got loading transition
				UILoadingTransition * loadingTransition = dynamic_cast<UILoadingTransition*> (transitionProcessed);
                DVASSERT(loadingTransition);

				// Firstly start transition
				loadingTransition->StartTransition(currentScreen, nextScreenProcessed);
				
				// Manage transfer to loading transition through InTransition of LoadingTransition
                if (loadingTransition->GetInTransition())
                {
                    loadingTransition->GetInTransition()->StartTransition(currentScreen, loadingTransition);
                    currentScreen = SafeRetain(loadingTransition->GetInTransition());
                }
                else 
                {
                    if(currentScreen)
                    {
                        if (currentScreen->IsOnScreen())
                            currentScreen->SystemWillBecomeInvisible();
                        currentScreen->SystemWillDisappear();
                        if ((nextScreenProcessed == 0) || (currentScreen->GetGroupId() != nextScreenProcessed->GetGroupId()))
                        {
                            currentScreen->UnloadGroup();
                        }
                        currentScreen->SystemDidDisappear();
                    }
                        // if we have next screen we load new resources, if it equal to zero we just remove screen
                    loadingTransition->LoadGroup();
                    loadingTransition->SystemWillAppear();
                    currentScreen = loadingTransition;
                    loadingTransition->SystemDidAppear();
                    if (loadingTransition->IsOnScreen())
                        loadingTransition->SystemWillBecomeVisible();
                }
			}
		}
        else	// if there is no transition do change immediatelly
		{	
			// if we have current screen we call events, unload resources for it group
			if(currentScreen)
			{
                if (currentScreen->IsOnScreen())
                    currentScreen->SystemWillBecomeInvisible();
				currentScreen->SystemWillDisappear();
				if ((nextScreenProcessed == 0) || (currentScreen->GetGroupId() != nextScreenProcessed->GetGroupId()))
				{
					currentScreen->UnloadGroup();
				}
				currentScreen->SystemDidDisappear();
			}
			// if we have next screen we load new resources, if it equal to zero we just remove screen
			if (nextScreenProcessed)
			{
				nextScreenProcessed->LoadGroup();
				nextScreenProcessed->SystemWillAppear();
			}
			currentScreen = nextScreenProcessed;
            NotifyListenersDidSwitch(currentScreen);
            if (nextScreenProcessed)
            {
				nextScreenProcessed->SystemDidAppear();
                if (nextScreenProcessed->IsOnScreen())
                    nextScreenProcessed->SystemWillBecomeVisible();
            }
			
			GetSystem<UIInputSystem>()->UnlockInput();
		}
		frameSkip = FRAME_SKIP;
		removeCurrentScreen = false;
	}
	
	/*
	 if we have popups to remove, we removes them here
	 */
	for (Set<UIPopup*>::iterator it = popupsToRemove.begin(); it != popupsToRemove.end(); it++)
	{
		UIPopup *p = *it;
		if (p) 
		{
			p->Retain();
			popupContainer->RemoveControl(p);
			p->UnloadGroup();
            p->Release();
		}
	}
	popupsToRemove.clear();
}

void UIControlSystem::Update()
{
	TIME_PROFILE("UIControlSystem::Update");

    updateCounter = 0;
	ProcessScreenLogic();
	
    GetSystem<UIUpdateSystem>()->Process();
	
	SafeRelease(prevScreen);
    //Logger::Info("UIControlSystem::updates: %d", updateCounter);
}
	
void UIControlSystem::Draw()
{
	GetSystem<UIRenderSystem>()->Process();
}
	
void UIControlSystem::ScreenSizeChanged()
{
    popupContainer->SystemScreenSizeDidChanged(VirtualCoordinatesSystem::Instance()->GetFullScreenVirtualRect());
}

void UIControlSystem::SetHoveredControl(UIControl *newHovered)
{
    if (hovered != newHovered) 
    {
        if (hovered) 
        {
            hovered->SystemDidRemoveHovered();
            hovered->Release();
        }
        hovered = SafeRetain(newHovered);
        if (hovered) 
        {
            hovered->SystemDidSetHovered();
        }
    }
}
    
UIControl *UIControlSystem::GetHoveredControl(UIControl *newHovered)
{
    return hovered;
}
    
void UIControlSystem::SetFocusedControl(UIControl *newFocused, bool forceSet)
{
    if (focusedControl)
    {
        if (forceSet || focusedControl->IsLostFocusAllowed(newFocused)) 
        {
            focusedControl->SystemOnFocusLost(newFocused);
            SafeRelease(focusedControl);
            focusedControl = SafeRetain(newFocused);
            if (focusedControl) 
            {
                focusedControl->SystemOnFocused();
            }
        }
    }
    else 
    {
        focusedControl = SafeRetain(newFocused);
        if (focusedControl) 
        {
            focusedControl->SystemOnFocused();
        }
    }

}
    
UIControl *UIControlSystem::GetFocusedControl()
{
    return focusedControl;
}

const UIGeometricData &UIControlSystem::GetBaseGeometricData() const
{
	return baseGeometricData;	
}

void UIControlSystem::ReplayEvents()
{
	while(Replay::Instance()->IsEvent())
	{
		int32 activeCount = Replay::Instance()->PlayEventsCount();
		Vector<UIEvent> activeInputs;
		while(activeCount--)
		{
			UIEvent ev = Replay::Instance()->PlayEvent();
			activeInputs.push_back(ev);
		}

		int32 allCount = Replay::Instance()->PlayEventsCount();
		Vector<UIEvent> allInputs;
		while(allCount--)
		{
			UIEvent ev = Replay::Instance()->PlayEvent();
			allInputs.push_back(ev);
		}

		if(activeCount || allCount)
		{
			GetSystem<UIInputSystem>()->OnInput(0, activeInputs, allInputs, true);
		}
	}
}

int32 UIControlSystem::LockSwitch()
{
	screenLockCount++;
	return screenLockCount;
}

int32 UIControlSystem::UnlockSwitch()
{
	screenLockCount--;
	DVASSERT(screenLockCount >= 0);
	return screenLockCount;
}

void UIControlSystem::AddScreenSwitchListener(ScreenSwitchListener * listener)
{
	screenSwitchListeners.push_back(listener);
}

void UIControlSystem::RemoveScreenSwitchListener(ScreenSwitchListener * listener)
{
	Vector<ScreenSwitchListener *>::iterator it = std::find(screenSwitchListeners.begin(), screenSwitchListeners.end(), listener);
	if(it != screenSwitchListeners.end())
		screenSwitchListeners.erase(it);
}

void UIControlSystem::NotifyListenersWillSwitch( UIScreen* screen )
{
    Vector<ScreenSwitchListener*> screenSwitchListenersCopy = screenSwitchListeners;
    uint32 listenersCount = (uint32)screenSwitchListenersCopy.size();
    for(uint32 i = 0; i < listenersCount; ++i)
        screenSwitchListenersCopy[i]->OnScreenWillSwitch( screen );
}

void UIControlSystem::NotifyListenersDidSwitch( UIScreen* screen )
{
    Vector<ScreenSwitchListener*> screenSwitchListenersCopy = screenSwitchListeners;
    uint32 listenersCount = (uint32)screenSwitchListenersCopy.size();
    for(uint32 i = 0; i < listenersCount; ++i)
        screenSwitchListenersCopy[i]->OnScreenDidSwitch( screen );
}


void UIControlSystem::UI3DViewAdded()
{
    ui3DViewCount++;
}
void UIControlSystem::UI3DViewRemoved()
{
    DVASSERT(ui3DViewCount);
    ui3DViewCount--;
}

};
