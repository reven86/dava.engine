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




#ifndef __DAVAENGINE_EDIT_MATRIX_CONTROL_H__
#define __DAVAENGINE_EDIT_MATRIX_CONTROL_H__

#include "DAVAEngine.h"

namespace DAVA 
{
class EditMatrixControl : public UIControl, public UITextFieldDelegate
{
protected:
    ~EditMatrixControl();
public:
    EditMatrixControl(const Rect & _rect, bool _readOnly = false);
    

    void SetMatrix(const Matrix4 & _matrix);
    const Matrix4 & GetMatrix() const;
    void OnEditButtonPressed(BaseObject * obj, void *, void *);
    void OnEditClosePressed(BaseObject * obj, void *, void *);

    
	void TextFieldShouldReturn(UITextField * textField);
	bool TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString);
    
    virtual bool IsTextFieldShouldSetFocusedOnAppear(UITextField * textField);


    virtual void Input(UIEvent * touch);
    
    Message OnMatrixChanged;

    void SetReadOnly(bool _readOnly);

protected:
    Matrix4 matrix;
    UITextField * textField;
    UIControl * textFieldBackground;
    UIButton * matrixButtons[4][4]; 
    int32 editI, editJ;
    void GetIndexByButton(BaseObject * button, int32 & i, int32 & j);
    
    bool readOnly;
};
};

#endif // __DAVAENGINE_EDIT_MATRIX_CONTROL_H__