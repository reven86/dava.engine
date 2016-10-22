#ifndef __DAVAENGINE_UI_CONTROL_SYSTEM_H__
#define __DAVAENGINE_UI_CONTROL_SYSTEM_H__

#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Base/Singleton.h"
#include "Base/TemplateHelpers.h"
#include "Engine/Private/EnginePrivateFwd.h"
#include "UI/UIControl.h"
#include "UI/UIEvent.h"
#include "UI/UIPopup.h"
#include "UI/UIScreenTransition.h"

#define FRAME_SKIP 5

/**
	\defgroup controlsystem	UI System
*/
namespace DAVA
{
class UIScreen;
class UISystem;
class UILayoutSystem;
class UIStyleSheetSystem;
class UIFocusSystem;
class UIInputSystem;
class UIScreenshoter;

class ScreenSwitchListener
{
public:
    virtual ~ScreenSwitchListener() = default;

    virtual void OnScreenWillSwitch(UIScreen* newScreen)
    {
    }

    virtual void OnScreenDidSwitch(UIScreen* newScreen)
    {
    }
};

/**
	 \brief	UIControlSystem it's a core of the all controls work.
		ControlSystem managed all update, draw, appearence and disappearence of the controls.
		ControlSystem works with th UIScreenManager to process screen setting and switching.
		Also ControlSystem processed all user input events to the controls.
	 */

class UIControlSystem : public Singleton<UIControlSystem>
{
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
    void SetScreen(UIScreen* newMainControl, UIScreenTransition* transition = 0);

    /**
	 \brief Sets the requested screen as current.
	 \returns currently seted screen
	 */
    UIScreen* GetScreen() const;

    /**
	 \brief Adds new popup to the popup container.
	 \param[in] Popup control to add.
	 */
    void AddPopup(UIPopup* newPopup);

    /**
	 \brief Removes popup from the popup container.
	 \param[in] Popup control to remove.
	 */
    void RemovePopup(UIPopup* newPopup);

    /**
	 \brief Removes all popups from the popup container.
	 */
    void RemoveAllPopups();

    /**
	 \brief Returns popups container.
		User can manage this container manually (change popup sequence, removes or adds popups)
	 \returns popup container
	 */
    UIControl* GetPopupContainer() const;

    UIScreenTransition* GetScreenTransition() const;

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
    void CancelInputs(UIControl* control, bool hierarchical = true);

    /**
	 \brief Cancel requested input.
	 \param[in] event to cancel.
	 */
    void CancelInput(UIEvent* touch);

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
    const Vector<UIEvent>& GetAllInputs() const;

    /**
	 \brief Sets requested control as a exclusive input locker.
	 All inputs goes only to the exclusive input locker if input locker is present.
	 \param[in] control to set the input locker.
	 \param[in] event id to cause a lock. All other events will be cancelled(excepts the locker == NULL situation).
	 */
    void SetExclusiveInputLocker(UIControl* locker, uint32 lockEventId);

    /**
	 \brief Returns current exclusive input locker. Returns NULL if exclusive input locker is not present.
	 \returns exclusive input locker
	 */
    UIControl* GetExclusiveInputLocker() const;

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
    const UIGeometricData& GetBaseGeometricData() const;

    /**
	 \brief Sets input with the requested ID to the required control.
		Input removes from the current owner. OnInputCancel() calls for the old control.  
		New control starts to handle all input activities.
	 \param[in] Input ID. Can be found in the UIEvent:touchId.
	 \param[in] Control that should handle the input.
	 */
    void SwitchInputToControl(uint32 eventID, UIControl* targetControl);

    /**
	 \brief Used internally by Replay class
	 */
    void ReplayEvents();

    /**
	 \brief Called by the core when screen size is changed
	 */
    void ScreenSizeChanged(const Rect& newFullscreenRect);

    /**
	 \brief Called by the control to set himself as the hovered control
	 */
    void SetHoveredControl(UIControl* newHovered);

    /**
	 \brief Returns control hovered by the mnouse for now
	 */
    UIControl* GetHoveredControl() const;

    /**
	 \brief Called by the control to set himself as the focused control
	 */
    void SetFocusedControl(UIControl* newFocused);

    /**
	 \brief Returns currently focused control
	 */
    UIControl* GetFocusedControl() const;

    void AddScreenSwitchListener(ScreenSwitchListener* listener);
    void RemoveScreenSwitchListener(ScreenSwitchListener* listener);

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

    bool IsBiDiSupportEnabled() const;
    void SetBiDiSupportEnabled(bool support);

    bool IsHostControl(const UIControl* control) const;

    void RegisterControl(UIControl* control);
    void UnregisterControl(UIControl* control);

    void RegisterVisibleControl(UIControl* control);
    void UnregisterVisibleControl(UIControl* control);

    void RegisterComponent(UIControl* control, UIComponent* component);
    void UnregisterComponent(UIControl* control, UIComponent* component);

    void AddSystem(std::unique_ptr<UISystem> sceneSystem, const UISystem* insertBeforeSystem = nullptr);
    std::unique_ptr<UISystem> RemoveSystem(const UISystem* sceneSystem);

    template <typename SystemClass>
    SystemClass* GetSystem() const
    {
        for (auto& system : systems)
        {
            if (DAVA::IsPointerToExactClass<SystemClass>(system.get()))
            {
                return static_cast<SystemClass*>(system.get());
            }
        }

        return nullptr;
    }

    UILayoutSystem* GetLayoutSystem() const;
    UIInputSystem* GetInputSystem() const;
    UIFocusSystem* GetFocusSystem() const;

    UIStyleSheetSystem* GetStyleSheetSystem() const;
    UIScreenshoter* GetScreenshoter();

    void SetClearColor(const Color& clearColor);
    void SetUseClearPass(bool useClearPass);

    void SetDefaultTapCountSettings();
    void SetTapCountSettings(float32 time, float32 inch);

    void UI3DViewAdded();
    void UI3DViewRemoved();
    int32 GetUI3DViewCount();

private:
    void ProcessScreenLogic();

    void NotifyListenersWillSwitch(UIScreen* screen);
    void NotifyListenersDidSwitch(UIScreen* screen);
    bool CheckTimeAndPosition(UIEvent* newEvent);
    int32 CalculatedTapCount(UIEvent* newEvent);

#if defined(__DAVAENGINE_COREV2__)
    friend class Private::EngineBackend;
#else
    friend void Core::CreateSingletons();
#endif

    Vector<std::unique_ptr<UISystem>> systems;
    UILayoutSystem* layoutSystem = nullptr;
    UIStyleSheetSystem* styleSheetSystem = nullptr;
    UIInputSystem* inputSystem = nullptr;
    UIScreenshoter* screenshoter = nullptr;

    Vector<ScreenSwitchListener*> screenSwitchListeners;

    RefPtr<UIScreen> currentScreen;
    RefPtr<UIScreen> nextScreen;
    RefPtr<UIScreenTransition> nextScreenTransition;
    RefPtr<UIScreenTransition> currentScreenTransition;
    RefPtr<UIControl> popupContainer;
    Set<UIPopup*> popupsToRemove;

    int32 lockInputCounter = 0;
    int32 screenLockCount = 0;
    int32 frameSkip = 0;

    UIGeometricData baseGeometricData;
    Rect fullscreenRect;

    bool removeCurrentScreen = false;

    uint32 resizePerFrame = 0; //used for logging some strange crahses on android
    float32 doubleClickRadiusSquared = 0.f;
    float32 doubleClickTime = 0.f;
    const float32 defaultDoubleClickTime = 0.5f; // seconds
    float32 defaultDoubleClickRadiusSquared = 0.f; // calculate in constructor
    struct LastClickData
    {
        uint32 touchId = 0;
        Vector2 physPoint;
        float64 timestamp = 0.0;
        int32 tapCount = 0;
        bool lastClickEnded = false;
    };
    LastClickData lastClickData;

    int32 ui3DViewCount = 0;
    bool needClearMainPass = true;
};
};

#endif
