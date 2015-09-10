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


#ifndef __DAVAENGINE_UI_EVENT_H__
#define __DAVAENGINE_UI_EVENT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"

namespace DAVA
{
	
class UIControl;
/**
\ingroup controlsystem
\brief User input representation.
	Represent user input event used in the control system. Contains all input data.
*/
class UIEvent
{
public:
    enum eInputSource
    {
        INPUT_UNDEFINED,
        INPUT_SOURCE_TOUCHSCREEN,
        INPUT_SOURCE_MOUSE,
        INPUT_SOURCE_KEYBOARD,
        INPUT_SOURCE_GAMEPAD
    };

    /**
	 \enum Control state bits.
	 */
    enum eInputPhase
	{
			PHASE_BEGAN = 0	//!<Screen touch or mouse button press is began.
		,	PHASE_DRAG		//!<User moves mouse with presset button or finger over the screen.
		,	PHASE_ENDED		//!<Screen touch or mouse button press is ended.
		,	PHASE_MOVE		//!<Mouse move event. Mouse moves without pressing any buttons. Works only with mouse controller.
        ,   PHASE_WHEEL     //!<Mouse wheel event. MacOS & Win32 only
		,	PHASE_CANCELLED	//!<Event was cancelled by the platform or by the control system for the some reason.  

		,	PHASE_KEYCHAR	//!<Event is a keyboard key pressing event.
		,	PHASE_JOYSTICK
	};
	
	/**
	 \enum Internal Control Sytem event activity state.
	 */
    enum eInputActivityState
    {
        ACTIVITY_STATE_INACTIVE = 0,
        ACTIVITY_STATE_ACTIVE,
        ACTIVITY_STATE_CHANGED
    };

    /**
	 \enum Input state accordingly to control.
	 */
    enum eControlInputState
	{
			CONTROL_STATE_RELEASED	=	0	//!<Input is released
		,	CONTROL_STATE_INSIDE			//!<Input processed inside control rerct for now
		,	CONTROL_STATE_OUTSIDE			//!<Input processed outside of the control rerct for now
	};
	
	/**
	 \ Input can be handled in the different ways.
	 */
	enum eInputHandledType
	{
		INPUT_NOT_HANDLED		= 0,//!<Input is not handled at all.
		INPUT_HANDLED_SOFT		= 1,//!<Input is handled, but input control can be changed by UIControlSystem::Instance()->SwitchInputToControl() method.
		INPUT_HANDLED_HARD		= 2,//!<Input is handled completely, input control can't be changed.
	};

	friend class UIControlSystem;

    enum eButtonID
    {
        BUTTON_NONE = 0,
        BUTTON_1,
        BUTTON_2,
        BUTTON_3
    };

    enum eJoystickAxisID
    {
        JOYSTICK_AXIS_X = 0,
        JOYSTICK_AXIS_Y,
        JOYSTICK_AXIS_Z,
        JOYSTICK_AXIS_RX,
        JOYSTICK_AXIS_RY,
        JOYSTICK_AXIS_RZ,
        JOYSTICK_AXIS_LTRIGGER,
        JOYSTICK_AXIS_RTRIGGER,
        JOYSTICK_AXIS_HAT_X,
        JOYSTICK_AXIS_HAT_Y
    };

    int32 tid; //!< event id, for the platforms with mouse this id means mouse button id, key codes for keys, axis id for joystick
    Vector2 point; //!< point of pressure in virtual coordinates
    Vector2 physPoint; //!< point of pressure in physical coordinates
    float64 timestamp; //!< time stemp of the event occurrence
    int32 phase; //!< began, ended, moved. See eInputPhase
    UIControl* touchLocker; //!< control that handles this input
    int32 activeState; //!< state of input in control system (active, inactive, changed)
    int32 controlState; //!< input state relative to control (outside, inside). Used for point inputs only(mouse, touch)
    int32 tapCount; //!< count of the continuous inputs (clicks for mouse)
    char16 keyChar; //!< unicode/translated character produced by key using current language, caps etc. Used only with PHASE_KEYCHAR.
    uint32 which; // index of mouse or touch surface in system

    inline void SetInputHandledType(eInputHandledType value)
    {
        // Input Handled Type can be only increased.
        if (inputHandledType < value)
        {
            inputHandledType = value;
        }
    }

    eInputHandledType GetInputHandledType() { return inputHandledType; };
    void ResetInputHandledType() { inputHandledType = INPUT_NOT_HANDLED; };

    UIEvent()
        : tid(0)
        , timestamp(0.f)
        , phase(0)
        , touchLocker(nullptr)
        , activeState(ACTIVITY_STATE_INACTIVE)
        , controlState(CONTROL_STATE_RELEASED)
        , tapCount(0)
        , keyChar(0)
        , which(0)
        , inputHandledType(INPUT_NOT_HANDLED)
    {
    }

    eInputSource GetInputSource()
    {
        switch (phase)
        {
            case PHASE_BEGAN:
            case PHASE_DRAG:
            case PHASE_ENDED:
            case PHASE_CANCELLED:
                return INPUT_SOURCE_TOUCHSCREEN;
            case PHASE_KEYCHAR:
                return INPUT_SOURCE_KEYBOARD;
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
			case PHASE_MOVE:
            case PHASE_WHEEL:
                return INPUT_SOURCE_MOUSE;
#endif
            case PHASE_JOYSTICK:
                return INPUT_SOURCE_GAMEPAD;
            default:
                return INPUT_UNDEFINED;
        }
    }

protected:
	eInputHandledType inputHandledType;//!< input handled type, INPUT_NOT_HANDLED by default. 
};
};

#endif
