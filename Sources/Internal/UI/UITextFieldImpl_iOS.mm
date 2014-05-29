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

#include "BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include <UIKit/UIKit.h>
#include "UI/UITextField.h"
#include "UI/UITextFieldImpl.h"
#include "Core/Core.h"

#import <HelperAppDelegate.h>

float GetUITextViewSizeDivider()
{
    float divider = 1.f;
    if (DAVA::Core::IsAutodetectContentScaleFactor()) 
    {
        if ([::UIScreen instancesRespondToSelector: @selector(scale) ]
            && [::UIView instancesRespondToSelector: @selector(contentScaleFactor) ]) 
        {
            divider = [[::UIScreen mainScreen] scale];
        }
    }
    
    return divider;
}
static const UITextAutocapitalizationType DavaToNativeUITextAutocapitalizationType[] = 
{
    UITextAutocapitalizationTypeNone,        //UITextField::AUTO_CAPITALIZATION_TYPE_NONE
    UITextAutocapitalizationTypeWords,       //UITextField::AUTO_CAPITALIZATION_TYPE_WORDS
    UITextAutocapitalizationTypeSentences,   //UITextField::AUTO_CAPITALIZATION_TYPE_SENTENCES
    UITextAutocapitalizationTypeAllCharacters//UITextField::AUTO_CAPITALIZATION_TYPE_ALL_CHARS
};

static const UITextAutocorrectionType DavaToNativeUITextAutocorrectionType[] = 
{
    UITextAutocorrectionTypeDefault,//UITextField::AUTO_CORRECTION_TYPE_DEFAULT
    UITextAutocorrectionTypeNo,     //UITextField::AUTO_CORRECTION_TYPE_NO
    UITextAutocorrectionTypeYes     //UITextField::AUTO_CORRECTION_TYPE_YES
};

static const UITextSpellCheckingType DavaToNativeUITextSpellCheckingType[] =
{
    UITextSpellCheckingTypeDefault, //UITextField::SPELL_CHECKING_TYPE_DEFAULT
    UITextSpellCheckingTypeNo,      //UITextField::SPELL_CHECKING_TYPE_NO
    UITextSpellCheckingTypeYes      //UITextField::SPELL_CHECKING_TYPE_YES
};

static const UIKeyboardAppearance DavaToNativeUIKeyboardAppearance[] =
{
    UIKeyboardAppearanceDefault,    //UITextField::KEYBOARD_APPEARANCE_DEFAULT
    UIKeyboardAppearanceAlert       //UITextField::KEYBOARD_APPEARANCE_ALERT
};

static const UIKeyboardType DavaToNativeUIKeyboardType[] =
{
    UIKeyboardTypeDefault,      //UITextField::KEYBOARD_TYPE_DEFAULT
    UIKeyboardTypeASCIICapable, //UITextField::KEYBOARD_TYPE_ASCII_CAPABLE
    UIKeyboardTypeNumbersAndPunctuation, //UITextField::KEYBOARD_TYPE_NUMBERS_AND_PUNCTUATION
    UIKeyboardTypeURL,          //UITextField::KEYBOARD_TYPE_URL
    UIKeyboardTypeNumberPad,    //UITextField::KEYBOARD_TYPE_NUMBER_PAD
    UIKeyboardTypePhonePad,     //UITextField::KEYBOARD_TYPE_PHONE_PAD
    UIKeyboardTypeNamePhonePad, //UITextField::KEYBOARD_TYPE_NAME_PHONE_PAD
    UIKeyboardTypeEmailAddress, //UITextField::KEYBOARD_TYPE_EMAIL_ADDRESS
    UIKeyboardTypeDecimalPad,   //UITextField::KEYBOARD_TYPE_DECIMAL_PAD
    UIKeyboardTypeTwitter       //UITextField::KEYBOARD_TYPE_TWITTER
};

static const UIReturnKeyType DavaToNativeUIReturnKeyType[] =
{
    UIReturnKeyDefault,         //UITextField::RETURN_KEY_DEFAULT
    UIReturnKeyGo,              //UITextField::RETURN_KEY_GO
    UIReturnKeyGoogle,          //UITextField::RETURN_KEY_GOOGLE
    UIReturnKeyJoin,            //UITextField::RETURN_KEY_JOIN
    UIReturnKeyNext,            //UITextField::RETURN_KEY_NEXT
    UIReturnKeyRoute,           //UITextField::RETURN_KEY_ROUTE
    UIReturnKeySearch,          //UITextField::RETURN_KEY_SEARCH
    UIReturnKeySend,            //UITextField::RETURN_KEY_SEND
    UIReturnKeyYahoo,           //UITextField::RETURN_KEY_YAHOO
    UIReturnKeyDone,            //UITextField::RETURN_KEY_DONE
    UIReturnKeyEmergencyCall    //UITextField::RETURN_KEY_EMERGENCY_CALL
};

@interface UITextFieldHolder : UIView < UITextFieldDelegate >
{
@public
	UITextField * textField;
	DAVA::UITextField * cppTextField;
	BOOL textInputAllowed;
    
    CGRect lastKeyboardFrame;
}
- (id) init : (DAVA::UITextField  *) tf;
- (void) dealloc;
- (BOOL)textFieldShouldReturn:(UITextField *)textField;
- (BOOL)textField:(UITextField *)_textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string;
- (void)setIsPassword:(bool)isPassword;
- (void)setTextInputAllowed:(bool)value;

- (void)setupTraits;

- (UITextAutocapitalizationType) convertAutoCapitalizationType:(DAVA::UITextField::eAutoCapitalizationType) davaType;
- (UITextAutocorrectionType) convertAutoCorrectionType:(DAVA::UITextField::eAutoCorrectionType) davaType;
- (UITextSpellCheckingType) convertSpellCheckingType:(DAVA::UITextField::eSpellCheckingType) davaType;
- (BOOL) convertEnablesReturnKeyAutomatically:(bool) davaType;
- (UIKeyboardAppearance) convertKeyboardAppearanceType:(DAVA::UITextField::eKeyboardAppearanceType) davaType;
- (UIKeyboardType) convertKeyboardType:(DAVA::UITextField::eKeyboardType) davaType;
- (UIReturnKeyType) convertReturnKeyType:(DAVA::UITextField::eReturnKeyType) davaType;

@end

@implementation UITextFieldHolder

- (id) init : (DAVA::UITextField  *) tf
{
	if (self = [super init])
	{
        float divider = GetUITextViewSizeDivider();
        
        self.bounds = CGRectMake(0.0f, 0.0f, DAVA::Core::Instance()->GetPhysicalScreenWidth()/divider, DAVA::Core::Instance()->GetPhysicalScreenHeight()/divider);
        
		self.center = CGPointMake(DAVA::Core::Instance()->GetPhysicalScreenWidth()/2/divider, DAVA::Core::Instance()->GetPhysicalScreenHeight()/2/divider);
		self.userInteractionEnabled = TRUE;
		textInputAllowed = YES;

		cppTextField = tf;
		DAVA::Rect rect = tf->GetRect();
		textField = [[UITextField alloc] initWithFrame: CGRectMake((rect.x - DAVA::Core::Instance()->GetVirtualScreenXMin()) 
                                                                   * DAVA::Core::GetVirtualToPhysicalFactor()
																   , (rect.y - DAVA::Core::Instance()->GetVirtualScreenYMin()) * DAVA::Core::GetVirtualToPhysicalFactor()
																   , rect.dx * DAVA::Core::GetVirtualToPhysicalFactor()
																   , rect.dy * DAVA::Core::GetVirtualToPhysicalFactor())];
		textField.frame = CGRectMake((rect.x - DAVA::Core::Instance()->GetVirtualScreenXMin()) * DAVA::Core::GetVirtualToPhysicalFactor()
									 , (rect.y - DAVA::Core::Instance()->GetVirtualScreenYMin()) * DAVA::Core::GetVirtualToPhysicalFactor()
									 , rect.dx * DAVA::Core::GetVirtualToPhysicalFactor()
									 , rect.dy * DAVA::Core::GetVirtualToPhysicalFactor());
        
		textField.delegate = self;
		
		[self setupTraits];
        
        textField.userInteractionEnabled = NO;

		// Done!
		[self addSubview:textField];
	}
	return self;
}

-(void)textFieldDidBeginEditing:(UITextField *)textField
{
    if (cppTextField)
    {
        if (DAVA::UIControlSystem::Instance()->GetFocusedControl() != cppTextField)
        {
            DAVA::UIControlSystem::Instance()->SetFocusedControl(cppTextField, false);
        }
    }
}

-(id)hitTest:(CGPoint)point withEvent:(UIEvent *)event
{
    id hitView = [super hitTest:point withEvent:event];
    if (hitView == self) return nil;
    else return hitView;
}

- (void) dealloc
{
	[textField release];
	textField = 0;
	[[NSNotificationCenter defaultCenter] removeObserver:self];

	[super dealloc];
}


- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
	if (cppTextField)
	{
		if (cppTextField->GetDelegate() != 0)
			cppTextField->GetDelegate()->TextFieldShouldReturn(cppTextField);
	}
	return TRUE;
}

- (BOOL)textField:(UITextField *)_textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
	if (cppTextField)
	{
		if (cppTextField->GetDelegate() != 0)
		{
			DAVA::WideString repString;
            const char * cstr = [string cStringUsingEncoding:NSUTF8StringEncoding];
            DAVA::UTF8Utils::EncodeToWideString((DAVA::uint8*)cstr, strlen(cstr), repString);
			return cppTextField->GetDelegate()->TextFieldKeyPressed(cppTextField, range.location, range.length, repString);
		}
	}
	return TRUE;
}

- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField
{
	return textInputAllowed;
}

- (void)setIsPassword:(bool)isPassword
{
	[textField setSecureTextEntry:isPassword ? YES: NO];
}

- (void)setTextInputAllowed:(bool)value
{
	textInputAllowed = (value == true);
}

- (void) setupTraits
{
	if (!cppTextField || !textField)
	{
		return;
	}

	textField.autocapitalizationType = [self convertAutoCapitalizationType: cppTextField->GetAutoCapitalizationType()];
	textField.autocorrectionType = [self convertAutoCorrectionType: cppTextField->GetAutoCorrectionType()];
	
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_5_0
	textField.spellCheckingType = [self convertSpellCheckingType: cppTextField->GetSpellCheckingType()];
#endif
	textField.enablesReturnKeyAutomatically = [self convertEnablesReturnKeyAutomatically: cppTextField->IsEnableReturnKeyAutomatically()];
	textField.keyboardAppearance = [self convertKeyboardAppearanceType: cppTextField->GetKeyboardAppearanceType()];
	textField.keyboardType = [self convertKeyboardType: cppTextField->GetKeyboardType()];
	textField.returnKeyType = [self convertReturnKeyType: cppTextField->GetReturnKeyType()];
}

- (UITextAutocapitalizationType) convertAutoCapitalizationType:(DAVA::UITextField::eAutoCapitalizationType) davaType
{
    return DavaToNativeUITextAutocapitalizationType[davaType];
}

- (UITextAutocorrectionType) convertAutoCorrectionType:(DAVA::UITextField::eAutoCorrectionType) davaType
{
    return DavaToNativeUITextAutocorrectionType[davaType];
}

- (UITextSpellCheckingType) convertSpellCheckingType:(DAVA::UITextField::eSpellCheckingType) davaType
{
    return DavaToNativeUITextSpellCheckingType[davaType];
}

- (BOOL) convertEnablesReturnKeyAutomatically:(bool) davaType
{
	return (davaType ? YES : NO);
}

- (UIKeyboardAppearance) convertKeyboardAppearanceType:(DAVA::UITextField::eKeyboardAppearanceType) davaType
{
    return DavaToNativeUIKeyboardAppearance[davaType];
}

- (UIKeyboardType) convertKeyboardType:(DAVA::UITextField::eKeyboardType) davaType
{
    return DavaToNativeUIKeyboardType[davaType];
}

- (UIReturnKeyType) convertReturnKeyType:(DAVA::UITextField::eReturnKeyType) davaType
{
    return DavaToNativeUIReturnKeyType[davaType];
}

@end

namespace DAVA 
{
    UITextFieldImpl::UITextFieldImpl(UITextField * tf)
    {
        UITextFieldHolder * textFieldHolder = [[UITextFieldHolder alloc] init: (DAVA::UITextField*)tf];
        nativeClassPtr = textFieldHolder;
    }
    UITextFieldImpl::~UITextFieldImpl()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
        [textFieldHolder removeFromSuperview];
        [textFieldHolder release];
        textFieldHolder = 0;
    }
	
    void UITextFieldImpl::GetTextColor(DAVA::Color &color) const
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
        [textFieldHolder->textField.textColor getRed:&color.r green:&color.g blue:&color.b alpha:&color.a];
    }
    
    void UITextFieldImpl::SetTextColor(const DAVA::Color &color)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
        textFieldHolder->textField.textColor = [UIColor colorWithRed:color.r green:color.g blue:color.b alpha:color.a];
    }
    
    Font *UITextFieldImpl::GetFont(){ return NULL; }
    void UITextFieldImpl::SetFont(Font * font){}
    
    void UITextFieldImpl::SetFontSize(float size)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
        float scaledSize = size * Core::GetVirtualToPhysicalFactor();
        
        if( [[::UIScreen mainScreen] respondsToSelector:@selector(displayLinkWithTarget:selector:)])
        {
            scaledSize /= [::UIScreen mainScreen].scale;
        }
        textFieldHolder->textField.font = [UIFont systemFontOfSize:scaledSize];
    }
    
    void UITextFieldImpl::SetTextAlign(DAVA::int32 align)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
        if (align & ALIGN_LEFT)
		{
            textFieldHolder->textField.contentHorizontalAlignment = UIControlContentHorizontalAlignmentLeft;
			textFieldHolder->textField.textAlignment = NSTextAlignmentLeft;
		}
        else if (align & ALIGN_HCENTER)
		{
            textFieldHolder->textField.contentHorizontalAlignment = UIControlContentHorizontalAlignmentCenter;
			textFieldHolder->textField.textAlignment = NSTextAlignmentCenter;
		}
        else if (align & ALIGN_RIGHT)
		{
            textFieldHolder->textField.contentHorizontalAlignment = UIControlContentHorizontalAlignmentRight;
			textFieldHolder->textField.textAlignment = NSTextAlignmentRight;
		}

        if (align & ALIGN_TOP)
            textFieldHolder->textField.contentVerticalAlignment = UIControlContentVerticalAlignmentTop;
        else if (align & ALIGN_VCENTER)
            textFieldHolder->textField.contentVerticalAlignment = UIControlContentVerticalAlignmentCenter;
        else if (align & ALIGN_BOTTOM)
            textFieldHolder->textField.contentVerticalAlignment = UIControlContentVerticalAlignmentBottom;
    }

    void UITextFieldImpl::OpenKeyboard()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
        textFieldHolder->textField.userInteractionEnabled = YES;
        [textFieldHolder->textField becomeFirstResponder];
    }
    
    void UITextFieldImpl::CloseKeyboard()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
        textFieldHolder->textField.userInteractionEnabled = NO;
        [textFieldHolder->textField resignFirstResponder];
    }
    
    void UITextFieldImpl::ShowField()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
        HelperAppDelegate* appDelegate = [[UIApplication sharedApplication] delegate];
        [[appDelegate glController].backgroundView addSubview:textFieldHolder];
    }
    
    void UITextFieldImpl::HideField()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
        [textFieldHolder removeFromSuperview];
    }
    
    void UITextFieldImpl::UpdateRect(const Rect & rect, float32 timeElapsed)
    {
        float divider = GetUITextViewSizeDivider();
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
        CGRect cgRect = CGRectMake((rect.x - DAVA::Core::Instance()->GetVirtualScreenXMin()) * DAVA::Core::GetVirtualToPhysicalFactor()/divider
                                   , (rect.y - DAVA::Core::Instance()->GetVirtualScreenYMin()) * DAVA::Core::GetVirtualToPhysicalFactor()/divider
                                   , rect.dx * DAVA::Core::GetVirtualToPhysicalFactor()/divider
                                   , rect.dy * DAVA::Core::GetVirtualToPhysicalFactor()/divider);
        textFieldHolder->textField.frame = cgRect;
    }
	
    void UITextFieldImpl::SetText(const WideString & string)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
        textFieldHolder->textField.text = [[ [ NSString alloc ]  
                                            initWithBytes : (char*)string.data()   
                                            length : string.size() * sizeof(wchar_t)   
                                            encoding : CFStringConvertEncodingToNSStringEncoding ( kCFStringEncodingUTF32LE ) ] autorelease];
        
        [textFieldHolder->textField.undoManager removeAllActions];
    }
	
    void UITextFieldImpl::GetText(WideString & string) const
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
        
        const char * cstr = [textFieldHolder->textField.text cStringUsingEncoding:NSUTF8StringEncoding];
        DAVA::UTF8Utils::EncodeToWideString((DAVA::uint8*)cstr, strlen(cstr), string);
    }

	void UITextFieldImpl::SetIsPassword(bool isPassword)
	{
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
		[textFieldHolder setIsPassword: isPassword];
	}

	void UITextFieldImpl::SetInputEnabled(bool value)
	{
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
		[textFieldHolder setTextInputAllowed:value];
	}

	void UITextFieldImpl::SetAutoCapitalizationType(DAVA::int32 value)
	{
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
		textFieldHolder->textField.autocapitalizationType = [textFieldHolder convertAutoCapitalizationType:
															 (DAVA::UITextField::eAutoCapitalizationType)value];
	}

	void UITextFieldImpl::SetAutoCorrectionType(DAVA::int32 value)
	{
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
		textFieldHolder->textField.autocorrectionType = [textFieldHolder convertAutoCorrectionType:
														 (DAVA::UITextField::eAutoCorrectionType)value];
	}

	void UITextFieldImpl::SetSpellCheckingType(DAVA::int32 value)
	{
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_5_0
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
		textFieldHolder->textField.spellCheckingType = [textFieldHolder convertSpellCheckingType:
														 (DAVA::UITextField::eSpellCheckingType)value];
#endif
	}

	void UITextFieldImpl::SetKeyboardAppearanceType(DAVA::int32 value)
	{
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
		textFieldHolder->textField.keyboardAppearance = [textFieldHolder convertKeyboardAppearanceType:
														(DAVA::UITextField::eKeyboardAppearanceType)value];
	}

	void UITextFieldImpl::SetKeyboardType(DAVA::int32 value)
	{
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
		textFieldHolder->textField.keyboardType = [textFieldHolder convertKeyboardType:
														 (DAVA::UITextField::eKeyboardType)value];
	}

	void UITextFieldImpl::SetReturnKeyType(DAVA::int32 value)
	{
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
		textFieldHolder->textField.returnKeyType = [textFieldHolder convertReturnKeyType:
												   (DAVA::UITextField::eReturnKeyType)value];
	}
	
	void UITextFieldImpl::SetEnableReturnKeyAutomatically(bool value)
	{
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
		textFieldHolder->textField.enablesReturnKeyAutomatically = [textFieldHolder convertEnablesReturnKeyAutomatically:value];
	}

    uint32 UITextFieldImpl::GetCursorPos() const
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
        if (!textFieldHolder)
        {
            return 0;
        }

        ::UITextField* textField = textFieldHolder->textField;
        int pos = [textField offsetFromPosition: textField.beginningOfDocument
                                     toPosition: textField.selectedTextRange.start];
        return pos;
    }

    void UITextFieldImpl::SetCursorPos(uint32 pos)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)nativeClassPtr;
        if (!textFieldHolder)
        {
            return;
        }

        ::UITextField* textField = textFieldHolder->textField;
        NSUInteger textLength = [textField.text length];
        if (textLength == 0)
        {
            return;
        }
        if (pos > textLength)
        {
            pos = textLength - 1;
        }

        UITextPosition *start = [textField positionFromPosition:[textField beginningOfDocument] offset:pos];
        UITextPosition *end = [textField positionFromPosition:start offset:0];
        [textField setSelectedTextRange:[textField textRangeFromPosition:start toPosition:end]];
    }

    void UITextFieldImpl::Input(UIEvent *currentInput){}
    void UITextFieldImpl::Draw(const UIGeometricData &geometricData){}
};

#endif
