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

#ifndef __DAVEENGINE_UI_INPUT_SYSTEM_H__
#define __DAVEENGINE_UI_INPUT_SYSTEM_H__

#include "UISystem.h"
#include "Entity/Component.h"

namespace DAVA
{
class UIControl;
class UIEvent;

class UIInputSystem : public UISystem
{
public:
    IMPLEMENT_SYSTEM_TYPE(UISystem::UI_INPUT_SYSTEM);
    IMPLEMENT_REQUIRED_COMPONENTS(MAKE_COMPONENT_MASK(Component::UI_INPUT_COMPONENT));

    UIInputSystem();
    virtual ~UIInputSystem();

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
    inline const Vector<UIEvent>& GetAllInputs() const;

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
    inline UIControl* GetExclusiveInputLocker() const;

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
    inline int32 GetLockInputCounter() const;


private:
    void CopyTouchData(UIEvent* dst, const UIEvent* src);

    Vector<UIEvent> totalInputs;
    UIControl *exclusiveInputLocker;
    int32 inputCounter;
    int32 lockInputCounter;


};

inline const Vector<UIEvent>& UIInputSystem::GetAllInputs() const
{
    return totalInputs;
}

inline int32 UIInputSystem::GetLockInputCounter() const
{
    return lockInputCounter;
}

inline UIControl* UIInputSystem::GetExclusiveInputLocker() const
{
    return exclusiveInputLocker;
}

}


#endif // __DAVEENGINE_UI_INPUT_SYSTEM_H__