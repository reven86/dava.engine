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



#ifndef __CREATE_PROPERTY_CONTROL_H__
#define __CREATE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "ComboBox.h"

using namespace DAVA;

class CreatePropertyControlDelegate
{
public:
    virtual void NodeCreated(bool success, const String &name, int32 type, VariantType *defaultValue = NULL) = 0;
};

class CreatePropertyControl: public UIControl, public UITextFieldDelegate, public ComboBoxDelegate
{
    
public:
    
    enum ePropertyType
    {
        EPT_STRING = 0,
        EPT_INT,
        EPT_FLOAT,
        EPT_BOOL,
        
        EPT_COUNT
    };
protected:
    virtual ~CreatePropertyControl();
public:
    CreatePropertyControl(const Rect & rect, CreatePropertyControlDelegate *newDelegate);
    
    virtual void WillAppear();

    virtual void TextFieldShouldReturn(UITextField * textField);
    virtual void TextFieldShouldCancel(UITextField * textField);
    virtual void TextFieldLostFocus(UITextField * textField);
    virtual bool TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString);
        
	virtual void OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex);

//     const String GetPropName() const;
//     int32 GetPropType() const;
            
protected:
	int32 GetValueTypeFromTypeIndex(int32 typeIndex);
	int32 GetTypeIndexFromValueType(int32 type);

	void UpdateEditableControls();

	UIStaticText *presetText;
	ComboBox *presetCombo;
	String emptyPresetName;
	String selectedPresetName;

    ComboBox *typeCombo;
    UITextField *nameField;

	bool isPresetMode;
        
    CreatePropertyControlDelegate *delegate;
        
    void OnCancel(BaseObject * object, void * userData, void * callerData);
    void OnCreate(BaseObject * object, void * userData, void * callerData);
};



#endif // __CREATE_PROPERTY_CONTROL_H__