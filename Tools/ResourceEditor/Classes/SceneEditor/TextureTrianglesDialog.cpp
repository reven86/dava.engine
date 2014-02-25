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



#include "TextureTrianglesDialog.h"
#include "ControlsFactory.h"
    
TextureTrianglesDialog::TextureTrianglesDialog()
    :   ExtendedDialog()
{
    workingPolygonGroup = NULL;
    previewSprite = NULL;

    Rect rect = GetDialogRect();
    draggableDialog->SetRect(rect);

    rect.x = rect.y = 0;
    
    Vector<String> comboboxItems;
    comboboxItems.push_back("0");
    comboboxItems.push_back("1");
    comboboxItems.push_back("2");
    comboboxItems.push_back("3");
    
    Rect comboRect = rect;
    comboRect.dy = ControlsFactory::BUTTON_HEIGHT;
    combobox = new ComboBox(comboRect, this, comboboxItems);
    draggableDialog->AddControl(combobox);
    
    Rect previewRect = rect;
    previewRect.y = ControlsFactory::BUTTON_HEIGHT;
    previewRect.dy -= ControlsFactory::BUTTON_HEIGHT * 2;
    texturePreview = new UIControl(previewRect);
    texturePreview->SetInputEnabled(false);
    draggableDialog->AddControl(texturePreview);
    
    float32 buttonX = (rect.dx - ControlsFactory::BUTTON_WIDTH) / 2;
    float32 buttonY = rect.dy - ControlsFactory::BUTTON_HEIGHT;
    btnCancel = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), LocalizedString(L"dialog.close"));
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &TextureTrianglesDialog::OnClose));
    draggableDialog->AddControl(btnCancel);
}

TextureTrianglesDialog::~TextureTrianglesDialog()
{
    SafeRelease(btnCancel);

    SafeRelease(workingPolygonGroup);
    SafeRelease(previewSprite);
    SafeRelease(texturePreview);
    SafeRelease(combobox);
}

const Rect TextureTrianglesDialog::GetDialogRect() const
{
    Rect rect = GetScreenRect();
    
    if(rect.dx < rect.dy)
    {
        rect.dy = rect.dx + ControlsFactory::BUTTON_HEIGHT * 2;
    }
    else
    {
        rect.dx = rect.dy - ControlsFactory::BUTTON_HEIGHT * 2;
    }
    
    rect.y = (GetScreenRect().dy - rect.dy) / 2;
    rect.x = (GetScreenRect().dx - rect.dx) / 2;
    
    return rect;
}

void TextureTrianglesDialog::UpdateSize()
{
    ExtendedDialog::UpdateSize();
    
    Rect rect = GetDialogRect();
    rect.x = rect.y = 0;
    
    Rect previewRect = rect;
    previewRect.y = ControlsFactory::BUTTON_HEIGHT;
    previewRect.dy -= ControlsFactory::BUTTON_HEIGHT * 2;
    texturePreview->SetRect(previewRect);

    float32 buttonX = (rect.dx - ControlsFactory::BUTTON_WIDTH) / 2;
    float32 buttonY = rect.dy - ControlsFactory::BUTTON_HEIGHT;
    btnCancel->SetPosition(Vector2(buttonX, buttonY));
}


void TextureTrianglesDialog::Close()
{
    SafeRelease(workingPolygonGroup);
    
    ExtendedDialog::Close();
}


void TextureTrianglesDialog::OnClose(BaseObject *, void *, void *)
{
    Close();
}


void TextureTrianglesDialog::Show(PolygonGroup *polygonGroup)
{
    if(!GetParent())
    {
        InitWithData(polygonGroup);
        
        UIScreenManager::Instance()->GetScreen()->AddControl(this);
    }
}

void TextureTrianglesDialog::InitWithData(PolygonGroup *pg)
{
    SafeRelease(workingPolygonGroup);
    workingPolygonGroup = SafeRetain(pg);
    
    Vector<String> comboboxItems;
    if(workingPolygonGroup)
    {
        for(int32 i = 0; i < workingPolygonGroup->textureCoordCount; ++i)
        {
            comboboxItems.push_back(Format("%d", i));
        }
    }
    else
    {
        comboboxItems.push_back("No coordinates");
    }
    combobox->SetNewItemsSet(comboboxItems);
    
    combobox->SetSelectedIndex(0, true);
}

void TextureTrianglesDialog::ConvertCoordinates(Vector2 &coordinates, const Vector2 &boxSize)
{
    coordinates.x = (coordinates.x + 0.5f) * boxSize.x;
    coordinates.y = (coordinates.y + 0.5f) * boxSize.y;
}


void TextureTrianglesDialog::FillRenderTarget(int32 textureIndex)
{
    SafeRelease(previewSprite);
    
    if(workingPolygonGroup)
    {
        const int32 IndexFlags[] = {EVF_TEXCOORD0, EVF_TEXCOORD1, EVF_TEXCOORD2, EVF_TEXCOORD3};
        
        if(workingPolygonGroup->vertexFormat & IndexFlags[textureIndex])
        {
            Vector2 usedSize = texturePreview->GetSize();
            previewSprite = Sprite::CreateAsRenderTarget(usedSize.x, usedSize.y, FORMAT_RGBA8888);
            
            RenderManager::Instance()->SetRenderTarget(previewSprite);
            RenderManager::Instance()->SetColor(Color::Black);
            RenderHelper::Instance()->FillRect(Rect(0, 0, usedSize.x, usedSize.y));
            
            usedSize.x = Min(usedSize.x, usedSize.y);
            usedSize.y = Min(usedSize.x, usedSize.y);
            
            usedSize /= 2.0f;
            
            RenderManager::Instance()->SetColor(Color(1.f, 0.f, 0.f, 1.f));
            Vector2 lt(0, 0), lb(0, 1), rt(1, 0), rb(1, 1);
            ConvertCoordinates(lt, usedSize);
            ConvertCoordinates(lb, usedSize);
            ConvertCoordinates(rt, usedSize);
            ConvertCoordinates(rb, usedSize);

            
            RenderHelper::Instance()->DrawLine(lt, lb);
            RenderHelper::Instance()->DrawLine(lt, rt);
            RenderHelper::Instance()->DrawLine(lb, rb);
            RenderHelper::Instance()->DrawLine(rt, rb);
            
            
            RenderManager::Instance()->SetColor(Color(0.f, 1.f, 0.f, 1.f));
            int32 indexCount = workingPolygonGroup->GetIndexCount();
            for(int32 i = 0; i < indexCount - 3; i += 3)
            {
                int32 index0 = 0, index1 = 0, index2 = 0;
                workingPolygonGroup->GetIndex(i + 0, index0);
                workingPolygonGroup->GetIndex(i + 1, index1);
                workingPolygonGroup->GetIndex(i + 2, index2);
                
                Vector2 v0, v1, v2;
                workingPolygonGroup->GetTexcoord(textureIndex, index0, v0);
                workingPolygonGroup->GetTexcoord(textureIndex, index1, v1);
                workingPolygonGroup->GetTexcoord(textureIndex, index2, v2);
             
                ConvertCoordinates(v0, usedSize);
                ConvertCoordinates(v1, usedSize);
                ConvertCoordinates(v2, usedSize);

                RenderHelper::Instance()->DrawLine(v0, v1);
                RenderHelper::Instance()->DrawLine(v1, v2);
                RenderHelper::Instance()->DrawLine(v0, v2);
            }
            
            
            RenderManager::Instance()->RestoreRenderTarget();
        }
    }
    
    texturePreview->SetSprite(previewSprite, 0);
}

void TextureTrianglesDialog::OnItemSelected(ComboBox *, const String &, int itemIndex)
{
    FillRenderTarget(itemIndex);
}

