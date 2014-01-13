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



#include "HintManager.h"
#include "ControlsFactory.h"

static const float32 NOTIFICATION_TIME = 3;

HintControl::HintControl(const Rect &rect, bool rectInAbsoluteCoordinates)
    :   UIControl(rect, rectInAbsoluteCoordinates)
{
    ControlsFactory::CusomizeBottomLevelControl(this);

    hintText = new UIStaticText();
    hintText->SetFont(ControlsFactory::GetFont12());
	hintText->SetTextColor(ControlsFactory::GetColorDark());
    hintText->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    hintText->SetText(L"");
    AddControl(hintText);
}

HintControl::~HintControl()
{
    SafeRelease(hintText);
}

void HintControl::SetText(const WideString &hintMessage)
{
    Size2i requestedSize = hintText->GetFont()->GetStringSize(hintMessage);
    Vector2 controlSize((float32)requestedSize.dx + 20.f, (float32)requestedSize.dy + 10.f);
    hintText->SetRect(Rect(0, 0, controlSize.dx, controlSize.dy));
    this->SetSize(controlSize);

    hintText->SetText(hintMessage);
}


HintManager::HintManager()
{
}
    
HintManager::~HintManager()
{
    for(int32 h = 0; h < (int32)hints.size(); ++h)
    {
        if(hints[h] && hints[h]->GetParent())
        {
            hints[h]->GetParent()->RemoveControl(hints[h]);
        }
        SafeRelease(hints[h]);
    }
    hints.clear();
}

void HintManager::ShowHint(const WideString &hintMessage, const DAVA::Rect &controlRectAbsolute)
{
    if(0 != hintMessage.length())
    {
        //Add control
        HintControl *hintControl = new HintControl();
        hintControl->SetText(hintMessage);
        
        Rect screenRect = UIScreenManager::Instance()->GetScreen()->GetRect(true);
        
        Vector2 requestedSize = hintControl->GetSize();
        if(requestedSize.dx < controlRectAbsolute.GetSize().dx)
        {
            Vector2 pos(controlRectAbsolute.x - screenRect.x, controlRectAbsolute.y - screenRect.y + controlRectAbsolute.dy);
            hintControl->SetPosition(pos);
        }
        else if(controlRectAbsolute.x - screenRect.x < requestedSize.dx)
        {
            Vector2 pos(controlRectAbsolute.x - screenRect.x, controlRectAbsolute.y - screenRect.y + controlRectAbsolute.dy);
            hintControl->SetPosition(pos);
        }
        else 
        {
            Vector2 pos(controlRectAbsolute.x - screenRect.x + controlRectAbsolute.dx - requestedSize.dx, controlRectAbsolute.y - screenRect.y + controlRectAbsolute.dy);
            hintControl->SetPosition(pos);
        }
        
        ControlsFactory::AddBorder(hintControl);
        UIScreenManager::Instance()->GetScreen()->AddControl(hintControl);
        
        Animation *hintAlphaAnimation = hintControl->ColorAnimation( Color::Transparent, NOTIFICATION_TIME, 
                                                                     Interpolation::EASY_IN, 2);
        hintAlphaAnimation->AddEvent(Animation::EVENT_ANIMATION_END, 
                                    Message(this, &HintManager::OnAlphaAnimationDone, hintControl));
        hints.push_back(hintControl);
    }
}

void HintManager::OnAlphaAnimationDone(BaseObject * owner, void * userData, void * callerData)
{
    UIControl *hintControl = (UIControl *)userData;

    for(Vector<HintControl*>::iterator it = hints.begin(); it != hints.end(); ++it)
    {
        if((*it) == hintControl && hintControl)
        {
            if(hintControl->GetParent())
            {
                hintControl->GetParent()->RemoveControl(hintControl);
            }
            
            SafeRelease(hintControl);
            hints.erase(it);
            break;
        }
    }
}


