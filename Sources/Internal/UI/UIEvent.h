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
#include "Math/Vector.h"

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
    enum class Phase : int32
    {
        ERROR = 0,
        BEGAN, //!<Screen touch or mouse button press is began.
        DRAG, //!<User moves mouse with presset button or finger over the screen.
        ENDED, //!<Screen touch or mouse button press is ended.
        MOVE, //!<Mouse move event. Mouse moves without pressing any buttons. Works only with mouse controller.
        WHEEL, //!<Mouse wheel event. MacOS & Win32 only
        CANCELLED, //!<(ios only)Event was cancelled by the platform or by the control system for the some reason.
        CHAR, //!<Event some symbol was intered.
        CHAR_REPEAT, //!< Usefull if User hold key in text editor and wait
        KEY_DOWN,
        KEY_DOWN_REPEAT, //!< Usefull if user hold key in text editor and wait cursor to move
        KEY_UP,
        PHASE_JOYSTICK
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
        CONTROL_STATE_RELEASED = 0, //!<Input is released
        CONTROL_STATE_INSIDE, //!<Input processed inside control rerct for now
        CONTROL_STATE_OUTSIDE //!<Input processed outside of the control rerct for now
    };

    /**
	 \ Input can be handled in the different ways.
	 */
    enum eInputHandledType
    {
        INPUT_NOT_HANDLED = 0, //!<Input is not handled at all.
        INPUT_HANDLED_SOFT = 1, //!<Input is handled, but input control can be changed by UIControlSystem::Instance()->SwitchInputToControl() method.
        INPUT_HANDLED_HARD = 2, //!<Input is handled completely, input control can't be changed.
    };

    friend class UIControlSystem;

    enum eButtonID : int32
    {
        BUTTON_NONE = 0,
        BUTTON_1,
        BUTTON_2,
        BUTTON_3
    };

    enum eJoystickAxisID : int32
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

    enum class Device : uint32
    {
        UNKNOWN = 0,
        TOUCH_SURFACE,
        MOUSE,
        KEYBOARD,
        GAMEPAD,
        PEN
    };

    UIEvent() = default;

    void SetInputHandledType(eInputHandledType value)
    {
        // Input Handled Type can be only increased.
        if (inputHandledType < value)
        {
            inputHandledType = value;
        }
    }

    eInputHandledType GetInputHandledType() { return inputHandledType; };
    void ResetInputHandledType() { inputHandledType = INPUT_NOT_HANDLED; };

    int32 tid = 0; // event id, for the platforms with mouse this id means mouse button id, key codes for keys, axis id for joystick
    Vector2 point; // point of pressure in virtual coordinates
    Vector2 physPoint; // point of pressure in physical coordinates
    float64 timestamp = 0.0; //(TODO not all platforms) time stemp of the event occurrence
    Phase phase = Phase::BEGAN; // began, ended, moved. See Phase
    UIControl* touchLocker = nullptr; // control that handles this input
    int32 activeState = ACTIVITY_STATE_INACTIVE; // state of input in control system (active, inactive, changed)
    int32 controlState = CONTROL_STATE_RELEASED; // input state relative to control (outside, inside). Used for point inputs only(mouse, touch)
    int32 tapCount = 0; // (TODO not all platforms) count of the continuous inputs (clicks for mouse)
    char16 keyChar = 0; // (TODO make char32_t) unicode/translated character produced by key using current language, caps etc. Used only with CHAR.
    Device device = Device::UNKNOWN;
    eInputHandledType inputHandledType = INPUT_NOT_HANDLED; //!< input handled type, INPUT_NOT_HANDLED by default.
};
};

#endif
