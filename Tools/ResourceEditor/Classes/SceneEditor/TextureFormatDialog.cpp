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



#include "TextureFormatDialog.h"

#include "ControlsFactory.h"
#include "UICheckBox.h"

#include "../Qt/Main/QtUtils.h"


TextureFormatDialog::TextureFormatDialog(TextureFormatDialogDelegate *newDelegate)
    :   ExtendedDialog()
    ,   delegate(newDelegate)
    ,   currentPVRButton(PVR_NONE)
{
    Rect rect = GetDialogRect();
    draggableDialog->SetRect(rect);
    
    //close
    float32 closeButtonSide = ControlsFactory::BUTTON_HEIGHT;
    closeButtonTop = ControlsFactory::CreateCloseWindowButton(Rect(rect.dx - closeButtonSide, 0, closeButtonSide, closeButtonSide));
    closeButtonTop->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &TextureFormatDialog::OnCancel));
    draggableDialog->AddControl(closeButtonTop);

    //convert
    float32 x = (rect.dx - ControlsFactory::BUTTON_WIDTH)/2;
    convertButton = ControlsFactory::CreateButton(Vector2(x, 
                                                          rect.dy - (float32)ControlsFactory::BUTTON_HEIGHT), 
                                                  LocalizedString(L"textureconverter.convert"));
    convertButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &TextureFormatDialog::OnConvert));
    
    draggableDialog->AddControl(convertButton);

    
    //PVR
    const WideString pvrNames[] = 
    {
        L"PVRTC4",
        L"PVRTC2"
    };
    
    float32 pvrX = (rect.dx - ControlsFactory::BUTTON_WIDTH * PVR_COUNT) / 2.f;
    float32 mipmapX = pvrX;
    for(int32 iPvr = 0; iPvr < PVR_COUNT; ++iPvr)
    {
        pvrButtons[iPvr] = ControlsFactory::CreateButton(Vector2(pvrX, ControlsFactory::BUTTON_HEIGHT), pvrNames[iPvr]);
        pvrButtons[iPvr]->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &TextureFormatDialog::OnPVRButton));
        draggableDialog->AddControl(pvrButtons[iPvr]);
        
        pvrX += ControlsFactory::BUTTON_WIDTH;
    }
    

    //MIPMAPS
    mipmapEnabled = new UICheckBox("~res:/Gfx/UI/chekBox", Rect(mipmapX, rect.dy - ControlsFactory::BUTTON_HEIGHT * 2.f, 
                                                               (float32)ControlsFactory::BUTTON_HEIGHT, (float32)ControlsFactory::BUTTON_HEIGHT));
    draggableDialog->AddControl(mipmapEnabled);
    
    Rect textRect;
    textRect.x = mipmapEnabled->GetPosition().x + mipmapEnabled->GetSize().x + ControlsFactory::OFFSET;
    textRect.y = mipmapEnabled->GetPosition().y;
    textRect.dx = mipmapEnabled->GetPosition().x + ControlsFactory::BUTTON_WIDTH * PVR_COUNT - textRect.x;
    textRect.dy = mipmapEnabled->GetSize().y;
    UIStaticText *t = new UIStaticText(textRect);
    t->SetFont(ControlsFactory::GetFont12());
	t->SetTextColor(ControlsFactory::GetColorLight());
    t->SetAlign(ALIGN_LEFT|ALIGN_VCENTER);
    t->SetText(LocalizedString(L"textureformatdialog.generatemipmap"));
    draggableDialog->AddControl(t);
    SafeRelease(t);
}


TextureFormatDialog::~TextureFormatDialog()
{
    for(int32 i = 0; i < PVR_COUNT; ++i)
    {
        SafeRelease(pvrButtons[i]);
    }
    SafeRelease(mipmapEnabled);

    SafeRelease(convertButton);
    SafeRelease(closeButtonTop);
}

void TextureFormatDialog::Show()
{
    if(!GetParent())
    {
        currentPVRButton = PVR_NONE;
        for(int32 i = 0; i < PVR_COUNT; ++i)
        {
            pvrButtons[i]->SetSelected(false);
        }
        mipmapEnabled->SetChecked(true, false);
        
        UIScreen *screen = UIScreenManager::Instance()->GetScreen();
        screen->AddControl(this);
    }
}


const Rect TextureFormatDialog::GetDialogRect() const
{
    Rect rect;
    
    rect.dx = ControlsFactory::BUTTON_WIDTH * 3;
    rect.dy = ControlsFactory::BUTTON_HEIGHT * 4;
    
    rect.x = (GetScreenRect().dx - rect.dx) / 2;
    rect.y = (GetScreenRect().dy - rect.dy) / 2;
    
    return rect;
}

void TextureFormatDialog::OnCancel(BaseObject *, void *, void *)
{
    Close();
}

void TextureFormatDialog::OnPVRButton(BaseObject * owner, void *, void *)
{
    UIButton *btn = (UIButton *)owner;
    for(int32 iPvr = 0; iPvr < PVR_COUNT; ++iPvr)
    {
        pvrButtons[iPvr]->SetSelected(false);
    }
    btn->SetSelected(true);
    
    for(int32 iPvr = 0; iPvr < PVR_COUNT; ++iPvr)
    {
        if(pvrButtons[iPvr] == btn)
        {
            currentPVRButton = (ePVRButtons)iPvr;
            break;
        }
    }

}

void TextureFormatDialog::OnConvert(DAVA::BaseObject *, void *, void *)
{
    if(PVR_NONE == currentPVRButton)
    {
        ShowErrorDialog(String("Format not selected."));
    }
    else 
    {
        if(delegate)
        {
            const PixelFormat formats[] =
            {
                FORMAT_PVR4,
                FORMAT_PVR2
            };
            delegate->OnFormatSelected(formats[currentPVRButton], mipmapEnabled->Checked());
        }
        
        OnCancel(NULL, NULL, NULL);
    }
}

