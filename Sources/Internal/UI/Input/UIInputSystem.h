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

#ifndef __DAVAENGINE_UI_INPUT_SYSTEM_H__
#define __DAVAENGINE_UI_INPUT_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Math/Vector.h"
#include "UI/Input/UIActionMap.h"
#include "UI/Input/UIInputMap.h"

namespace DAVA
{
class UIControl;
class UIScreen;
class UIEvent;
class UIFocusSystem;
class UIKeyInputSystem;

class UIInputSystem
{
public:
    UIInputSystem();
    ~UIInputSystem();

    void SetCurrentScreen(UIScreen* screen);
    void SetPopupContainer(UIControl* popupContainer);

    void OnControlVisible(UIControl* control);
    void OnControlInvisible(UIControl* control);

    void HandleEvent(UIEvent* event);

    void CancelInput(UIEvent* touch);
    void CancelAllInputs();
    void CancelInputs(UIControl* control, bool hierarchical);
    void SwitchInputToControl(uint32 eventID, UIControl* targetControl);

    const Vector<UIEvent>& GetAllInputs() const;

    void SetExclusiveInputLocker(UIControl* locker, uint32 lockEventId);
    UIControl* GetExclusiveInputLocker() const;
    void SetHoveredControl(UIControl* newHovered);
    UIControl* GetHoveredControl() const;

    UIFocusSystem* GetFocusSystem() const;
    UIKeyInputSystem* GetKeyInputSystem() const;

private:
    void UpdateModalControl();
    UIControl* FindNearestToUserModalControl() const;
    UIControl* FindNearestToUserModalControlImpl(UIControl* current) const;

    UIScreen* currentScreen = nullptr;
    UIControl* popupContainer = nullptr;
    RefPtr<UIControl> modalControl;

    UIFocusSystem* focusSystem = nullptr;
    UIKeyInputSystem* keyInputSystem = nullptr;

    UIControl* hovered = nullptr;

    Vector<UIEvent> touchEvents;
    UIControl* focusedControlWhenTouchBegan = nullptr;
    Vector2 positionOfTouchWhenTouchBegan;
    UIControl* exclusiveInputLocker = nullptr;
};
}


#endif //__DAVAENGINE_UI_KEY_INPUT_SYSTEM_H__
