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


#ifndef __DAVAENGINE_UI_TEXT_FIELD_HOLDER_H__
#define __DAVAENGINE_UI_TEXT_FIELD_HOLDER_H__

#include "Base/BaseTypes.h"

#if defined (__DAVAENGINE_IPHONE__)

#import <UIKit/UIKit.h>
#import "UI/UITextField.h"

@interface UITextFieldHolder : UIView < UITextFieldDelegate, UITextViewDelegate >
{
    NSString * cachedText;
@public
    // hold single line text field if user switch to multiline mode
    // otherwise nullptr
    UITextField* textField;
    // hold UITextField(singleline) or UITextView(multiline)
    UIView*    textCtrl;
    DAVA::UITextField * cppTextField;
    BOOL textInputAllowed;
    BOOL useRtlAlign;
    
    CGRect lastKeyboardFrame;
}

- (void) setTextField:(DAVA::UITextField *) tf;
- (id) init;
- (void) dealloc;
- (BOOL)textFieldShouldReturn:(UITextField *)textField;
- (BOOL)textField:(UITextField *)_textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string;
- (BOOL)textView:(UITextView *)textView_ shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)string;

- (void)dropCachedText;
- (void)setIsPassword:(bool)isPassword;
- (void)setTextInputAllowed:(bool)value;
- (void)setUseRtlAlign:(bool)value;

- (void)eventEditingChanged:(UIView *)sender;
- (void)textViewDidChange:(UITextView*)textView;

- (void)setupTraits;

- (UITextAutocapitalizationType) convertAutoCapitalizationType:(DAVA::UITextField::eAutoCapitalizationType) davaType;
- (UITextAutocorrectionType) convertAutoCorrectionType:(DAVA::UITextField::eAutoCorrectionType) davaType;
- (UITextSpellCheckingType) convertSpellCheckingType:(DAVA::UITextField::eSpellCheckingType) davaType;
- (BOOL) convertEnablesReturnKeyAutomatically:(bool) davaType;
- (UIKeyboardAppearance) convertKeyboardAppearanceType:(DAVA::UITextField::eKeyboardAppearanceType) davaType;
- (UIKeyboardType) convertKeyboardType:(DAVA::UITextField::eKeyboardType) davaType;
- (UIReturnKeyType) convertReturnKeyType:(DAVA::UITextField::eReturnKeyType) davaType;

@end


#endif //#if defined (__DAVAENGINE_IPHONE__)

#endif // __DAVAENGINE_UI_TEXT_FIELD_HOLDER_H__
