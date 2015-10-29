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


#ifndef __DAVAENGINE_UI_CONTROL_SYSTEM_H__
#define __DAVAENGINE_UI_CONTROL_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/Singleton.h"
#include "UI/UIControl.h"
#include "UI/UIEvent.h"
#include "UI/UIScreenTransition.h"
#include "UI/UILoadingTransition.h"
#include "UI/UIPopup.h"
#include "Base/FastName.h"

#define FRAME_SKIP	5

/**
	\defgroup controlsystem	UI System
*/
namespace DAVA
{

class UIScreen;
class UILayoutSystem;
class UIStyleSheetSystem;
class UIScreenshoter;

class ScreenSwitchListener
{
public:
	virtual void OnScreenWillSwitch(UIScreen* newScreen) {}
    virtual void OnScreenDidSwitch(UIScreen* newScreen) {}
};

	/**
	 \brief	UIControlSystem it's a core of the all controls work.
		ControlSystem managed all update, draw, appearence and disappearence of the controls.
		ControlSystem works with th UIScreenManager to process screen setting and switching.
		Also ControlSystem processed all user input events to the controls.
	 */

extern const FastName FRAME_QUERY_UI_DRAW;

class UIControlSystem : public Singleton<UIControlSystem>
{
	friend void Core::CreateSingletons();
	
	int frameSkip;
	int transitionType;

    Vector<UIEvent> touchEvents;

protected:
	~UIControlSystem();
	/**
	 \brief Don't call this constructor!
	 */
	UIControlSystem();
			
public:
    /* 
       Player + 6 ally bots. All visible on the screen
    Old measures:
    UIControlSystem::inputs: 309
    UIControlSystem::updates: 310
    UIControlSystem::draws: 310

    New measures:
    UIControlSystem::inputs: 42
    */
    int32 updateCounter;
    int32 drawCounter;
    int32 inputCounter;

	/**
	 \brief Sets the requested screen as current.
		Screen will be seted only on the next frame.
		Previous seted screen will be removed.
	 \param[in] Screen you want to set as current
	 \param[in] Transition you want to use for the screen setting.
	 */
	void SetScreen(UIScreen *newMainControl, UIScreenTransition * transition = 0);

	/**
	 \brief Sets the requested screen as current.
	 \returns currently seted screen
	 */
	UIScreen *GetScreen();

	/**
	 \brief Adds new popup to the popup container.
	 \param[in] Popup control to add.
	 */
	void AddPopup(UIPopup *newPopup);

	/**
	 \brief Removes popup from the popup container.
	 \param[in] Popup control to remove.
	 */
	void RemovePopup(UIPopup *newPopup);

	/**
	 \brief Removes all popups from the popup container.
	 */
	void RemoveAllPopups();
	
	/**
	 \brief Returns popups container.
		User can manage this container manually (change popup sequence, removes or adds popups)
	 \returns popup container
	 */
	UIControl *GetPopupContainer();
	
	/**
	 \brief Disabled all controls inputs.
		Locking all inputs if input is unlocked or incrementing lock counter.
	 \returns current lock input counter
	 */
	int32 LockInput();
	
	/**
	 \brief Enabling all controls inputs.
	 Decrementing lock counter if counter is zero unlocking all inputs.
	 \returns current lock input counter
	 */
	int32 UnlockInput();
	
	/**
	 \brief Returns lock input counter.
	 \returns current lock input counter
	 */
	int32 GetLockInputCounter() const;

	/**
	 \brief Cancel all inputs for the requested control.
	 \param[in] control to cancel inputs for.
	 */
	void CancelInputs(UIControl *control, bool hierarchical = true);

	/**
	 \brief Cancel requested input.
	 \param[in] event to cancel.
	 */
	void CancelInput(UIEvent *touch);

	/**
	 \brief Cancelling all current inputs.
	 */
	void CancelAllInputs();
	
	/**
	 \brief Sets the current screen to 0 LOL.
	 */
	void Reset();
	/**
	 \brief Calls by the system for input processing.
	 */
    void OnInput(UIEvent* newEvent);

    /**
	 \brief Callse very frame by the system for update.
	 */
    void Update();

    /**
	 \brief Calls every frame by the system for draw.
		Draws all controls hierarchy to the screen.
	 */
    void Draw();

//	void SetTransitionType(int newTransitionType);
	
			
	/**
	 \brief Returns all currently active inputs.
	 \returns all inputs active in the system
	 */
	const Vector<UIEvent>  &GetAllInputs(); 
	
	/**
	 \brief Sets requested control as a exclusive input locker.
	 All inputs goes only to the exclusive input locker if input locker is present.
	 \param[in] control to set the input locker.
	 \param[in] event id to cause a lock. All other events will be cancelled(excepts the locker == NULL situation).
	 */
	void SetExclusiveInputLocker(UIControl *locker, int32 lockEventId);

	/**
	 \brief Returns current exclusive input locker. Returns NULL if exclusive input locker is not present.
	 \returns exclusive input locker
	 */
	UIControl *GetExclusiveInputLocker();

	/**
	 \brief Returns base geometric data seted in the system.
		Base GeometricData is usually has parameters looks a like:
		baseGeometricData.position = Vector2(0, 0);
		baseGeometricData.size = Vector2(0, 0);
		baseGeometricData.pivotPoint = Vector2(0, 0);
		baseGeometricData.scale = Vector2(1.0f, 1.0f);
		baseGeometricData.angle = 0;
		But system can change this parameters for the 
		specific device capabilities.
	 
	 \returns GeometricData uset for the base draw
	 */
	const UIGeometricData &GetBaseGeometricData() const;
	
	/**
	 \brief Sets input with the requested ID to the required control.
		Input removes from the current owner. OnInputCancel() calls for the old control.  
		New control starts to handle all input activities.
	 \param[in] Input ID. Can be found in the UIEvent:tid.
	 \param[in] Control that should handle the input.
	 */
	void SwitchInputToControl(int32 eventID, UIControl *targetControl);

	/**
	 \brief Used internally by Replay class
	 */
	void ReplayEvents();

	/**
	 \brief Called by the core when screen size is changed
	 */
    void ScreenSizeChanged();


	/**
	 \brief Called by the control to set himself as the hovered control
	 */
    void SetHoveredControl(UIControl *newHovered);

	/**
	 \brief Returns control hovered by the mnouse for now
	 */
    UIControl *GetHoveredControl(UIControl *newHovered);

    /**
	 \brief Called by the control to set himself as the focused control
	 */
    void SetFocusedControl(UIControl *newFocused, bool forceSet);
    
	/**
	 \brief Returns currently focused control
	 */
    UIControl *GetFocusedControl();
	
	void AddScreenSwitchListener(ScreenSwitchListener * listener);
	void RemoveScreenSwitchListener(ScreenSwitchListener * listener);

	/**
	 \brief Disallow screen switch.
	 Locking screen switch or incrementing lock counter.
	 \returns current screen switch lock counter
	 */
	int32 LockSwitch();

	/**
	 \brief Allow screen switch.
	 Decrementing lock counter if counter is zero unlocking screen switch.
	 \returns current screen switch lock counter
	 */
	int32 UnlockSwitch();
    
    bool IsRtl() const;
    void SetRtl(bool rtl);
    UILayoutSystem *GetLayoutSystem() const;
    UIStyleSheetSystem* GetStyleSheetSystem() const;
    UIScreenshoter* GetScreenshoter();

    void SetClearColor(const Color& clearColor);

private:
	/**
	 \brief Instantly replace one screen to enother.
		Call this only on your own risk if you are really know what you need. 
		May cause to abnormal behavior!
		Internally used by UITransition.
	 \param[in] Screen you want to set as current.
	 */
	void ReplaceScreen(UIScreen *newMainControl);

	void ProcessScreenLogic();

    void NotifyListenersWillSwitch( UIScreen* screen );
    void NotifyListenersDidSwitch( UIScreen* screen );

    UILayoutSystem *layoutSystem;
    UIStyleSheetSystem* styleSheetSystem;
    UIScreenshoter* screenshoter;

    Vector<ScreenSwitchListener*> screenSwitchListeners;

	UIScreen * currentScreen;
	UIScreen * nextScreen;
	UIScreen * prevScreen;

	int32 screenLockCount;

	bool removeCurrentScreen;
	
	UIControl *exclusiveInputLocker;
    UIControl *hovered;
    
    UIControl *focusedControl;

	UIControl * popupContainer;
	Set<UIPopup*> popupsToRemove;
	
	int32 lockInputCounter;
	
	UIScreenTransition * nextScreenTransition;
	
	UIGeometricData baseGeometricData;

    Color clearColor;

    friend class UIScreenTransition;
	friend class UIScreenManager;
};
};

#endif
