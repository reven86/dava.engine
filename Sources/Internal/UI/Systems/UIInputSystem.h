#ifndef __DAVEENGINE_UI_INPUT_SYSTEM_H__
#define __DAVEENGINE_UI_INPUT_SYSTEM_H__

#include "UISystem.h"
#include "UI/UIControl.h"

namespace DAVA
{

class UIInputSystem : public UISystem
{
public:
    static const uint32 TYPE = UISystem::UI_INPUT_SYSTEM;

    UIInputSystem();
    virtual ~UIInputSystem();

    virtual uint32 GetRequiredComponents() const override;
    virtual uint32 GetType() const override;
    virtual void Process() override;

    /**
    \brief Calls on every input event. Calls SystemInput() for all control children.
    If no one of the children is processed input. Calls ProcessInput() for the current control.
    Internal method used by ControlSystem.
    \param[in] currentInput Input information.
    \returns true if control processed this input.
    */
    bool SystemInput(UIControl* control, UIEvent *currentInput);
    /**
    \brief Process incoming input and if it's necessary calls Input() method for the control.
    Internal method used by ControlSystem.
    \param[in] currentInput Input information.
    \returns true if control processed this input.
    */
    bool SystemProcessInput(UIControl* control, UIEvent *currentInput);// Internal method used by ControlSystem
    /**
    \brief Calls when input processd by control is cancelled.
    Internal method used by ControlSystem.
    \param[in] currentInput Input information.
    */
    void SystemInputCancelled(UIControl* control, UIEvent *currentInput);



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

    /// TODO: touchType to be removed?
    /**
    \brief Calls by the system for input processing.
    */
    void OnInput(int32 touchType, const Vector<UIEvent> &activeInputs, const Vector<UIEvent> &allInputs, bool fromReplay = false);
    void OnInput(UIEvent * event);

    /**
    \brief Returns all currently active inputs.
    \returns all inputs active in the system
    */
    const Vector<UIEvent>  &GetAllInputs();

    /**
    \brief Sets input with the requested ID to the required control.
    Input removes from the current owner. OnInputCancel() calls for the old control.
    New control starts to handle all input activities.
    \param[in] Input ID. Can be found in the UIEvent:tid.
    \param[in] Control that should handle the input.
    */
    void SwitchInputToControl(int32 eventID, UIControl *targetControl);

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


private:
    Vector<UIEvent> totalInputs;
    UIControl *exclusiveInputLocker;
    int32 inputCounter;
    int32 lockInputCounter;


};

}


#endif // __DAVEENGINE_UI_INPUT_SYSTEM_H__