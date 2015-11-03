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


#ifndef __MULTILINETEST_TEST_H__
#define __MULTILINETEST_TEST_H__

#include "Infrastructure/BaseScreen.h"

namespace DAVA
{
    class UITextField;
}

class TextDelegate1;
class TextDelegate2;
class TextDelegateMulti;

class MultilineTest : public BaseScreen, public DAVA::UITextFieldDelegate
{
public:
    MultilineTest();

    void LoadResources() override;
    void UnloadResources() override;
    
    // UITextFieldDelegate interface
    void TextFieldOnTextChanged(DAVA::UITextField * textField, const DAVA::WideString& newText, const DAVA::WideString& oldText) override {}
    
private:
    void OnShowHideClick(BaseObject* sender, void * data, void * callerData);
    UIControl * topLayerControl = nullptr;

    DAVA::UIButton* CreateUIButton(DAVA::Font* font, const DAVA::Rect& rect, const DAVA::String& text,
                             void (MultilineTest::*onClick)(DAVA::BaseObject*, void*, void*));

    DAVA::UITextField* textField1 = nullptr;
    DAVA::UITextField* textField2 = nullptr;
    DAVA::UITextField* textFieldMulti = nullptr;

    TextDelegate1* textDelegate1 = nullptr;
    TextDelegate2* textDelegate2 = nullptr;
};

#endif //__MULTILINETEST_TEST_H__
