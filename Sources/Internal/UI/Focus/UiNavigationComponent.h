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

#ifndef __DAVAENGINE_UI_NAVIGATION_COMPONENT_H__
#define __DAVAENGINE_UI_NAVIGATION_COMPONENT_H__

#include "Base/BaseTypes.h"

#include "UI/Components/UIComponent.h"
#include "UI/Focus/FocusHelpers.h"

namespace DAVA
{
class UINavigationComponent : public UIBaseComponent<UIComponent::NAVIGATION_COMPONENT>
{
public:
    enum Direction
    {
        LEFT = 0,
        RIGHT,
        UP,
        DOWN,

        DIRECTION_COUNT
    };

    UINavigationComponent();
    UINavigationComponent(const UINavigationComponent& src);

protected:
    virtual ~UINavigationComponent();

private:
    UINavigationComponent& operator=(const UINavigationComponent&) = delete;

public:
    UINavigationComponent* Clone() const override;

    const String& GetNextFocusLeft() const;
    void SetNextFocusLeft(const String& val);

    const String& GetNextFocusRight() const;
    void SetNextFocusRight(const String& val);

    const String& GetNextFocusUp() const;
    void SetNextFocusUp(const String& val);

    const String& GetNextFocusDown() const;
    void SetNextFocusDown(const String& val);

    const String& GetNextControlPathInDirection(Direction dir);

private:
    String nextFocusPath[DIRECTION_COUNT];

public:
    INTROSPECTION_EXTEND(UINavigationComponent, UIComponent,
                         PROPERTY("left", "Next Focus Left", GetNextFocusLeft, SetNextFocusLeft, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("right", "Next Focus Right", GetNextFocusRight, SetNextFocusRight, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("up", "Next Focus Up", GetNextFocusUp, SetNextFocusUp, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("down", "Next Focus Down", GetNextFocusDown, SetNextFocusDown, I_SAVE | I_VIEW | I_EDIT));
};
}


#endif //__DAVAENGINE_UI_NAVIGATION_COMPONENT_H__
