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


#include "InputSystem.h"
#include "Input/KeyboardDevice.h"
#include "Input/GamepadDevice.h"
#include "UI/UIControlSystem.h"
#include "Render/RenderManager.h"

namespace DAVA 
{

InputSystem::InputSystem() :
keyboard(0),
gamepad(0),
isMultitouchEnabled(true)
{
    keyboard = new KeyboardDevice();
    gamepad = new GamepadDevice();
    AddInputCallback(InputCallback(UIControlSystem::Instance(), &UIControlSystem::OnInput, INPUT_DEVICE_KEYBOARD));
    pinCursor = false;
}
    
InputSystem::~InputSystem()
{
    SafeRelease(gamepad);
    SafeRelease(keyboard);
}

void InputSystem::ProcessInputEvent(UIEvent * event)
{
	for(Vector<InputCallback>::iterator it = callbacks.begin(); it != callbacks.end(); it++)
	{
        if (event->phase == UIEvent::Phase::CHAR && ((*it).devices & INPUT_DEVICE_KEYBOARD))
            (*it)(event);
        else if (event->phase == UIEvent::Phase::JOYSTICK && ((*it).devices & INPUT_DEVICE_JOYSTICK))
            (*it)(event);
		else if(((*it).devices & INPUT_DEVICE_TOUCH))
			(*it)(event);
	}
}

void InputSystem::AddInputCallback(const InputCallback& inputCallback)
{
	callbacks.push_back(inputCallback);
}

bool InputSystem::RemoveInputCallback(const InputCallback& inputCallback)
{
	Vector<InputCallback>::iterator it = find(callbacks.begin(), callbacks.end(), inputCallback);
	if(it != callbacks.end())
		callbacks.erase(it);
	else
		return false;

	return true;
}

void InputSystem::RemoveAllInputCallbacks()
{
	callbacks.clear();
}

void InputSystem::OnBeforeUpdate()
{
    keyboard->OnBeforeUpdate();
}
    
void InputSystem::OnAfterUpdate()
{
    keyboard->OnAfterUpdate();
}

void InputSystem::SetCursorPining(bool isPin)
{
    pinCursor = isPin;

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
    Cursor::SetCursorPinning(isPin);
#endif
}


};
