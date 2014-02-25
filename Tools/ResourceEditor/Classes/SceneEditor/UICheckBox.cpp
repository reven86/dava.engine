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



#include "UICheckBox.h"

REGISTER_CLASS(UICheckBox);

UICheckBox::UICheckBox()
	:   UIControl()
{
	checked = false;
    checkboxDelegate = NULL;

    GetBackground()->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    
    AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UICheckBox::OnClick));
}

UICheckBox::UICheckBox(const FilePath &spriteName, const Rect &rect, bool rectInAbsoluteCoordinates)
    :   UIControl(rect, rectInAbsoluteCoordinates)
{
    checkboxDelegate = NULL;
    
    GetBackground()->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    
    SetSprite(spriteName, 0);
    SetChecked(false, false);

    AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UICheckBox::OnClick));
}

void UICheckBox::LoadFromYamlNode( const YamlNode * node, UIYamlLoader * loader )
{
	UIControl::LoadFromYamlNode(node, loader);
}

void UICheckBox::LoadFromYamlNodeCompleted()
{
    UIControl::LoadFromYamlNodeCompleted();

	SetChecked(checked, false);
}

void UICheckBox::SetChecked( bool _checked, bool needDelegateCall)
{
	checked = _checked;

    if(GetSprite())
    {
        SetSpriteFrame((checked) ? 1 : 0);
    }
    
    if(checkboxDelegate && needDelegateCall)
    {
        checkboxDelegate->ValueChanged(this, checked);
    }
}

bool UICheckBox::Checked()
{
	return checked;
}

void UICheckBox::OnClick( BaseObject * owner, void * userData, void * callerData )
{
	SetChecked(!checked, true);
}

void UICheckBox::SetDelegate(UICheckBoxDelegate *delegate)
{
    checkboxDelegate = delegate;
}