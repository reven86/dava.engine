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


#ifndef __DAVAENGINE_INPUT_SYSTEM_H__
#define __DAVAENGINE_INPUT_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/Singleton.h"
#include "Core/Core.h"
#include "UI/UIEvent.h"
#include "InputCallback.h"
//#include "UI/UIControl.h"
//#include "UI/UIScreenTransition.h"
//#include "UI/UILoadingTransition.h"
//#include "UI/UIPopup.h"

/**
	\defgroup inputsystem	Input System
*/
namespace DAVA
{

class KeyboardDevice;
class GamepadDevice;

class InputSystem : public Singleton<InputSystem>
{
public:
	enum eInputDevice
	{
		INPUT_DEVICE_TOUCH		= 1,
		INPUT_DEVICE_KEYBOARD	= 1 << 1,
		INPUT_DEVICE_JOYSTICK	= 1 << 2
	};

    enum eMouseCaptureMode
    {
        MOUSE_CAPTURE_OFF = 0, //!< Disable any capturing (send absolute xy)
        MOUSE_CAPTURE_FRAME, //!< Capture system cursor into window rect (send absolute xy)
        MOUSE_CAPTURE_PINING //!<< Capture system cursor on current position (send xy move delta)
    };

    friend void Core::CreateSingletons();
	
protected:
	~InputSystem();
	/**
	 \brief Don't call this constructor!
	 */
	InputSystem();
			
public:
    
	void ProcessInputEvent(UIEvent * event);

	void AddInputCallback(const InputCallback& inputCallback);
	bool RemoveInputCallback(const InputCallback& inputCallback);
	void RemoveAllInputCallbacks();
    
    void OnBeforeUpdate();
    void OnAfterUpdate();
    
    inline KeyboardDevice & GetKeyboard();
    inline GamepadDevice  & GetGamepadDevice();

    eMouseCaptureMode GetCursorCaptureMode();
    bool SetCursorCaptureMode(eMouseCaptureMode mode);

    inline void EnableMultitouch(bool enabled);
    inline bool GetMultitouchEnabled() const;
    
protected:
    
    KeyboardDevice *keyboard;
    GamepadDevice *gamepad;

    Vector<InputCallback> callbacks;
    bool pinCursor;
    
    bool isMultitouchEnabled;
};

inline KeyboardDevice & InputSystem::GetKeyboard()
{
    return *keyboard;
}

inline GamepadDevice & InputSystem::GetGamepadDevice()
{
    return *gamepad;
}

inline void InputSystem::EnableMultitouch(bool enabled)
{
	isMultitouchEnabled = enabled;
}

inline bool InputSystem::GetMultitouchEnabled() const
{
	return isMultitouchEnabled;
}

};

#endif
