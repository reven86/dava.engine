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


#ifndef __DAVAENGINE_UI_SWITCH_H__
#define __DAVAENGINE_UI_SWITCH_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include "UI/UIButton.h"

namespace DAVA
{

class TogglePositionAnimation;

/*
 * buttonLeft.relativePosition.x and buttonRight.relativePosition.x mark movement bounds for 'toggle': it does mean that toggle never leaves 
 * space between pivot points of these controls. So if you want to have toggle that moves between the left edge of 'buttonLeft' and 
 * the right edge of 'buttonRight' set buttonLeft.pivotPoint.x = 0 and buttonRight.pivotPoint.x = buttonRight.size.dx.
 *
 * Default sizes for buttonLeft, buttonRight and toggle are set only once in UISwitch ctor and only if you provide 'rect' argument.
 * After this they never adjust their sizes and it's your responsibility to control this parameter.
 *
 * Don't use GetButtonXXX()->GetSelected() to determine what side is currently selected: GetIsLeftSelected() is what you need.
 */
class UISwitch : public UIControl
{

    friend class TogglePositionAnimation;
protected:
    virtual ~UISwitch();
public:
    UISwitch(const Rect &rect = Rect());

    virtual void LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader);
    virtual void LoadFromYamlNodeCompleted();
    virtual void CopyDataFrom(DAVA::UIControl *srcControl);

	YamlNode * SaveToYamlNode(UIYamlLoader * loader);

    virtual void AddControl(UIControl *control) override;
    virtual void RemoveControl(UIControl *control) override;
    virtual UISwitch *Clone() override;

    virtual void Input(UIEvent *currentInput) override;

    bool GetIsLeftSelected() const {return isLeftSelected;}
    void SetIsLeftSelected(bool aIsLeftSelected);

    UIButton * GetButtonNext() const {return buttonLeft.Get();}
    UIButton * GetButtonPrevious() const { return buttonRight.Get(); }
    UIButton * GetToggle() const { return toggle.Get(); }

    /*
     * If tap on any place beside toggle must provoke switch of controls state.
     */
    void SetSwitchOnTapBesideToggle(bool aSwitchOnTapBesideToggle) {switchOnTapBesideToggle = aSwitchOnTapBesideToggle;}
    bool GetSwitchOnTapBesideToggle() const {return switchOnTapBesideToggle;}

protected:
    void InternalSetIsLeftSelected(bool aIsLeftSelected, bool changeVisualState, UIEvent *inputEvent = NULL);
    void InitControls();
    void ReleaseControls();

    float32 GetToggleUttermostPosition();
    void CheckToggleSideChange(UIEvent *inputEvent = NULL);
    void ChangeVisualState();

    RefPtr<UIButton> buttonLeft;
    RefPtr<UIButton> buttonRight;
    RefPtr<UIButton> toggle;

	// Boolean variables are grouped together because of DF-2149.
    bool switchOnTapBesideToggle : 1;
    bool isLeftSelected : 1;

public:
    INTROSPECTION_EXTEND(UISwitch, UIControl,
        nullptr
        );
};

}
#endif //__DAVAENGINE_UI_SWITCH_H__