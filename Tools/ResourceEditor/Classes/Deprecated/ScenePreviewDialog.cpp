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


#include "ScenePreviewDialog.h"

#include "Deprecated/ControlsFactory.h"
#include "Qt/Settings/SettingsManager.h"
#include "ScenePreviewControl.h"

ScenePreviewDialog::ScenePreviewDialog()
    : ExtendedDialog()
    , preview(nullptr)
    , errorMessage(nullptr)
    , clickableBackgound(nullptr)
{
    UpdateSize();

    clickableBackgound.reset(new UIControl());
    clickableBackgound->SetInputEnabled(true, true);
    clickableBackgound->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ScenePreviewDialog::OnClose));

    preview.reset(new ScenePreviewControl(Rect(0, 0, ControlsFactory::PREVIEW_PANEL_HEIGHT, ControlsFactory::PREVIEW_PANEL_HEIGHT)));
    preview->SetDebugDraw(true);

    errorMessage.reset(new UIStaticText(preview->GetRect()));
    errorMessage->SetMultiline(true);
    errorMessage->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
	errorMessage->SetTextColor(ControlsFactory::GetColorError());
    errorMessage->SetFont(ControlsFactory::GetFont20());

    ScopedPtr<UIButton> button(ControlsFactory::CreateButton(Rect(0, ControlsFactory::PREVIEW_PANEL_HEIGHT,
                                                                  ControlsFactory::PREVIEW_PANEL_HEIGHT, ControlsFactory::BUTTON_HEIGHT),
                                                             LocalizedString(L"dialog.close")));
    button->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ScenePreviewDialog::OnClose));
    draggableDialog->AddControl(button);
}
    
ScenePreviewDialog::~ScenePreviewDialog()
{
}


void ScenePreviewDialog::Show(const FilePath &scenePathname)
{
    bool enabled = SettingsManager::GetValue(Settings::General_PreviewEnabled).AsBool();
    if(!enabled)
        return;
    
    if(!GetParent())
    {
        UIScreen *screen = UIScreenManager::Instance()->GetScreen();
        clickableBackgound->SetSize(screen->GetSize());
        clickableBackgound->SetPosition(Vector2(0, 0));
        screen->AddControl(clickableBackgound);
        screen->AddControl(this);

        screen->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ScenePreviewDialog::OnClose));
    }
    
    //show preview
    if(errorMessage->GetParent())
    {
        draggableDialog->RemoveControl(errorMessage);
    }
    
    int32 error = preview->OpenScene(scenePathname);
    if(SceneFileV2::ERROR_NO_ERROR == error)
    {
        if(!preview->GetParent())
        {
            draggableDialog->AddControl(preview);
        }
    }
    else
    {
        switch (error)
        {
            case SceneFileV2::ERROR_FAILED_TO_CREATE_FILE:
            {
                errorMessage->SetText(LocalizedString(L"library.errormessage.failedtocreeatefile"));
                break;
            }
                
            case SceneFileV2::ERROR_FILE_WRITE_ERROR:
            {
                errorMessage->SetText(LocalizedString(L"library.errormessage.filewriteerror"));
                break;
            }
                
            case SceneFileV2::ERROR_VERSION_IS_TOO_OLD:
            {
                errorMessage->SetText(LocalizedString(L"library.errormessage.versionistooold"));
                break;
            }
                
            case ScenePreviewControl::ERROR_CANNOT_OPEN_FILE:
            {
                errorMessage->SetText(LocalizedString(L"library.errormessage.cannotopenfile"));
                break;
            }
                
            case ScenePreviewControl::ERROR_WRONG_EXTENSION:
            {
                errorMessage->SetText(LocalizedString(L"library.errormessage.wrongextension"));
                break;
            }
                
            default:
                errorMessage->SetText(LocalizedString(L"library.errormessage.unknownerror"));
                break;
        }
        
        draggableDialog->AddControl(errorMessage);
    }
}


void ScenePreviewDialog::OnClose(BaseObject *, void *, void *)
{
    Close();
}


void ScenePreviewDialog::Close()
{
    UIControl* backgourndParent = clickableBackgound->GetParent();
    if (backgourndParent != nullptr)
    {
        backgourndParent->RemoveControl(clickableBackgound);
    }

    preview->ReleaseScene();
    preview->RecreateScene();

    ExtendedDialog::Close();
}

const Rect ScenePreviewDialog::GetDialogRect() const
{
    Rect screenRect = GetScreenRect();
    
    float32 x = (screenRect.dx - ControlsFactory::PREVIEW_PANEL_HEIGHT);
    float32 h = ControlsFactory::PREVIEW_PANEL_HEIGHT + ControlsFactory::BUTTON_HEIGHT;
    float32 y = (screenRect.dy - h) / 2;
    
    return Rect(x, y, ControlsFactory::PREVIEW_PANEL_HEIGHT, h);
}


void ScenePreviewDialog::UpdateSize()
{
	Rect dialogRect = GetDialogRect();
	SetRect(dialogRect);
    
	dialogRect.x = dialogRect.y = 0;
    draggableDialog->SetRect(dialogRect);
}
