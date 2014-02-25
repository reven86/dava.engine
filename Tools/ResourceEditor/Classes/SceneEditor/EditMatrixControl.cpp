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





#include "EditMatrixControl.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"

namespace DAVA 
{

EditMatrixControl::EditMatrixControl(const Rect & _rect, bool _readOnly)
    :   UIControl(_rect)
    ,   readOnly(_readOnly)
    ,   editI(-1)
    ,   editJ(-1)

{
    //UIYamlLoader::Load(this, "~res:/Screens/Panels/camera_panel.yaml");
    //UIText
    
    Font *f = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
    f->SetSize(12);
    
    float32 spacing = 2.0f;
    float32 buttonWidth = (_rect.dx - spacing * 2.0f) / 4.0f;
    float32 buttonHeight = (_rect.dy - spacing * 2.0f) / 4.0f;
    for (int32 i = 0; i < 4; ++i)
        for (int32 j = 0; j < 4; ++j)
        {
            matrixButtons[i][j] = new UIButton(Rect(spacing + i * buttonWidth, spacing + j * buttonHeight, buttonWidth, buttonHeight));
            
            matrixButtons[i][j]->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
            matrixButtons[i][j]->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.0f, 0.0f, 0.0f, 0.5f));
            matrixButtons[i][j]->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
            matrixButtons[i][j]->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.5f, 0.5f, 0.5f, 0.5f));
            matrixButtons[i][j]->SetStateDrawType(UIControl::STATE_HOVER, UIControlBackground::DRAW_FILL);
            matrixButtons[i][j]->GetStateBackground(UIControl::STATE_HOVER)->SetColor(Color(0.2f, 0.2f, 0.2f, 0.2f));
            matrixButtons[i][j]->SetStateFont(UIControl::STATE_NORMAL, f);
            matrixButtons[i][j]->SetStateText(UIControl::STATE_NORMAL, L"0.00000");
            matrixButtons[i][j]->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditMatrixControl::OnEditButtonPressed));
            AddControl(matrixButtons[i][j]);
        }
    
    textFieldBackground = new UIControl(Rect(0, 0, _rect.dx, _rect.dy));
    textFieldBackground->GetBackground()->SetColor(Color(0.0f, 0.0f, 0.0f, 0.7f));
    textFieldBackground->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    textFieldBackground->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EditMatrixControl::OnEditClosePressed));
    //textFieldBackground->SetInputEnabled(false);
    
    textField = new UITextField(Rect(spacing, spacing, buttonWidth * 4.0f, buttonHeight));
    textField->SetFont(f);
    textField->SetDelegate(this);

    SafeRelease(f);
}
        

EditMatrixControl::~EditMatrixControl()
{
	SafeRelease(textField);
	SafeRelease(textFieldBackground);

    for (int32 i = 0; i < 4; ++i)
        for (int32 j = 0; j < 4; ++j)
        {
            SafeRelease(matrixButtons[i][j]);
        }
}
    
void EditMatrixControl::GetIndexByButton(BaseObject * button, int32 & io, int32 & jo)
{
    io = jo = -1;
    
    for (int32 i = 0; i < 4; ++i)
        for (int32 j = 0; j < 4; ++j)
            if (button == matrixButtons[i][j])
            {
                io = i;
                jo = j;
                return;
            }
}

void EditMatrixControl::OnEditButtonPressed(BaseObject * obj, void *, void *)
{
    if (!readOnly)
    {
        GetIndexByButton(obj, editI, editJ);
        if ((editI != -1) && (editJ != -1))
        {
            textField->SetText(Format(L"%0.5f", matrix._data[editI][editJ]));
        }
        
        AddControl(textFieldBackground);
        AddControl(textField);
    }
}
                                  
void EditMatrixControl::OnEditClosePressed(BaseObject * obj, void *, void *)
{
    if (textFieldBackground->GetParent())
    {
        RemoveControl(textFieldBackground);
        RemoveControl(textField);
    }
}

void EditMatrixControl::SetMatrix(const Matrix4 & _matrix)
{
    matrix = _matrix;
    for (int32 i = 0; i < 4; ++i)
        for (int32 j = 0; j < 4; ++j)
        {
            matrixButtons[i][j]->SetStateText(UIControl::STATE_NORMAL, Format(L"%0.5f", matrix._data[i][j]));
        }
}
    
const Matrix4 & EditMatrixControl::GetMatrix() const
{
    return matrix;
}
    
void EditMatrixControl::TextFieldShouldReturn(UITextField * textField)
{
    if (textFieldBackground->GetParent())
    {
        String value = WStringToString(textField->GetText());
        if ((editI != -1) && (editJ != -1))
        {
            matrix._data[editI][editJ] = (float32)atof(value.c_str());
            editI = editJ = -1;
        }
        
        RemoveControl(textFieldBackground);
        RemoveControl(textField);
        
        OnMatrixChanged(this, 0);
        
        SetMatrix(matrix);
    }
}
    
bool EditMatrixControl::TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString)
{
    
    return true;
}

bool EditMatrixControl::IsTextFieldShouldSetFocusedOnAppear(UITextField * textField)
{
    return true;
}

    
void EditMatrixControl::SetReadOnly(bool _readOnly)
{
    readOnly = _readOnly;
}
    
void EditMatrixControl::Input(DAVA::UIEvent *event)
{    
    if(UIEvent::PHASE_KEYCHAR == event->phase)
    {
        if(DVKEY_ESCAPE == event->tid)
        {
            if(UIControlSystem::Instance()->GetFocusedControl() == textField ||
               UIControlSystem::Instance()->GetFocusedControl() == textFieldBackground ||
               UIControlSystem::Instance()->GetFocusedControl() == this)
            {
                OnEditClosePressed(NULL, NULL, NULL);
            }
        }
    }        
    UIControl::Input(event);
}

    
};
