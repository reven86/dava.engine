/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include <UIKit/UIKit.h>
#include "UI/UITextField.h"
#include "UI/UITextFieldiPhone.h"
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
            // TODO convert range?
            
            /*
            int length = [string length];
			repString.resize(length); 
			for (int i = 0; i < length; i++) 
			{
				unichar uchar = [string characterAtIndex:i];
				repString[i] = (wchar_t)uchar;
			}
             */
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
	switch (davaType)
	{
		case DAVA::UITextField::AUTO_CAPITALIZATION_TYPE_NONE:
		{
			return UITextAutocapitalizationTypeNone;
		}

		case DAVA::UITextField::AUTO_CAPITALIZATION_TYPE_WORDS:
		{
			return UITextAutocapitalizationTypeWords;
		}

		case DAVA::UITextField::AUTO_CAPITALIZATION_TYPE_ALL_CHARS:
		{
			return UITextAutocapitalizationTypeAllCharacters;
		}

		case DAVA::UITextField::AUTO_CAPITALIZATION_TYPE_SENTENCES:
		default:
		{
			// This is default one for iOS.
			return UITextAutocapitalizationTypeSentences;
		}
	}
}

- (UITextAutocorrectionType) convertAutoCorrectionType:(DAVA::UITextField::eAutoCorrectionType) davaType
{
	switch (davaType)
	{
		case DAVA::UITextField::AUTO_CORRECTION_TYPE_NO:
		{
			return UITextAutocorrectionTypeNo;
		}
			
		case DAVA::UITextField::AUTO_CORRECTION_TYPE_YES:
		{
			
			return UITextAutocorrectionTypeYes;
		}

		case DAVA::UITextField::AUTO_CORRECTION_TYPE_DEFAULT:
		default:
		{
			return UITextAutocorrectionTypeDefault;
		}
	}
}

- (UITextSpellCheckingType) convertSpellCheckingType:(DAVA::UITextField::eSpellCheckingType) davaType
{
	switch (davaType)
	{
		case DAVA::UITextField::SPELL_CHECKING_TYPE_NO:
		{
			return UITextSpellCheckingTypeNo;
		}

		case DAVA::UITextField::SPELL_CHECKING_TYPE_YES:
		{
			return UITextSpellCheckingTypeYes;
		}

		case DAVA::UITextField::SPELL_CHECKING_TYPE_DEFAULT:
		default:
		{
			return UITextSpellCheckingTypeDefault;
		}
	}
}

- (BOOL) convertEnablesReturnKeyAutomatically:(bool) davaType
{
	return (davaType ? YES : NO);
}

- (UIKeyboardAppearance) convertKeyboardAppearanceType:(DAVA::UITextField::eKeyboardAppearanceType) davaType
{
	switch (davaType)
	{
		case DAVA::UITextField::KEYBOARD_APPEARANCE_ALERT:
		{
			return UIKeyboardAppearanceAlert;
		}
			
		case DAVA::UITextField::KEYBOARD_APPEARANCE_DEFAULT:
		default:
		{
			return UIKeyboardAppearanceDefault;
		}
	}
}

- (UIKeyboardType) convertKeyboardType:(DAVA::UITextField::eKeyboardType) davaType
{
	switch (davaType)
	{
		case DAVA::UITextField::KEYBOARD_TYPE_ASCII_CAPABLE:
		{
			return UIKeyboardTypeASCIICapable;
		}

		case DAVA::UITextField::KEYBOARD_TYPE_NUMBERS_AND_PUNCTUATION:
		{
			return UIKeyboardTypeNumbersAndPunctuation;
		}

		case DAVA::UITextField::KEYBOARD_TYPE_URL:
		{
			return UIKeyboardTypeURL;
		}

		case DAVA::UITextField:: KEYBOARD_TYPE_NUMBER_PAD:
		{
			return UIKeyboardTypeNumberPad;
		}

		case DAVA::UITextField::KEYBOARD_TYPE_PHONE_PAD:
		{
			return UIKeyboardTypePhonePad;
		}

		case DAVA::UITextField::KEYBOARD_TYPE_NAME_PHONE_PAD:
		{
			return UIKeyboardTypeNamePhonePad;
		}

		case DAVA::UITextField::KEYBOARD_TYPE_EMAIL_ADDRESS:
		{
			return UIKeyboardTypeEmailAddress;
		}

		case DAVA::UITextField::KEYBOARD_TYPE_DECIMAL_PAD:
		{
			return UIKeyboardTypeDecimalPad;
		}

		case DAVA::UITextField::KEYBOARD_TYPE_TWITTER:
		{
			return UIKeyboardTypeTwitter;
		}

		case DAVA::UITextField::KEYBOARD_TYPE_DEFAULT:
		default:
		{
			return UIKeyboardTypeDefault;
		}
	}
}

- (UIReturnKeyType) convertReturnKeyType:(DAVA::UITextField::eReturnKeyType) davaType
{
	switch (davaType)
	{
		case DAVA::UITextField::RETURN_KEY_GO:
		{
			return UIReturnKeyGo;
		}

		case DAVA::UITextField::RETURN_KEY_GOOGLE:
		{
			return UIReturnKeyGoogle;
		}

		case DAVA::UITextField::RETURN_KEY_JOIN:
		{
			return UIReturnKeyJoin;
		}

		case DAVA::UITextField::RETURN_KEY_NEXT:
		{
			return UIReturnKeyNext;
		}

		case DAVA::UITextField::RETURN_KEY_ROUTE:
		{
			return UIReturnKeyRoute;
		}

		case DAVA::UITextField::RETURN_KEY_SEARCH:
		{
			return UIReturnKeySearch;
		}

		case DAVA::UITextField::RETURN_KEY_SEND:
		{
			return UIReturnKeySend;
		}

		case DAVA::UITextField::RETURN_KEY_YAHOO:
		{
			return UIReturnKeyYahoo;
		}

		case DAVA::UITextField::RETURN_KEY_DONE:
		{
			return UIReturnKeyDone;
		}

		case DAVA::UITextField::RETURN_KEY_EMERGENCY_CALL:
		{
			return UIReturnKeyEmergencyCall;
		}

		case DAVA::UITextField::RETURN_KEY_DEFAULT:
		default:
		{
			return UIReturnKeyDefault;
		}
	}
}

- (void)keyboardFrameDidChange:(NSNotification *)notification
{
    NSDictionary* userInfo = notification.userInfo;

    // Remember the last keyboard frame here, since it might be incorrect in keyboardDidShow.
    lastKeyboardFrame = [[notification.userInfo objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
}

- (void)keyboardWillHide:(NSNotification *)notification
{
	if (cppTextField && cppTextField->GetDelegate())
	{
		cppTextField->GetDelegate()->OnKeyboardHidden();
	}
}

- (void)keyboardDidShow:(NSNotification *)notification
{
	if (!cppTextField || !cppTextField->GetDelegate())
	{
		return;
	}

	// convert own frame to window coordinates, frame is in superview's coordinates
	CGRect ownFrame = [textField.window convertRect:self.frame fromView:textField.superview];

	// calculate the area of own frame that is covered by keyboard
	CGRect keyboardFrame = CGRectIntersection(ownFrame, lastKeyboardFrame);

	// now this might be rotated, so convert it back
	keyboardFrame = [textField.window convertRect:keyboardFrame toView:textField.superview];

	// Recalculate to virtual coordinates.
	DAVA::Vector2 keyboardOrigin(keyboardFrame.origin.x, keyboardFrame.origin.y);
	keyboardOrigin *= DAVA::UIControlSystem::Instance()->GetScaleFactor();
	keyboardOrigin += DAVA::UIControlSystem::Instance()->GetInputOffset();
	
	DAVA::Vector2 keyboardSize(keyboardFrame.size.width, keyboardFrame.size.height);
	keyboardSize *= DAVA::UIControlSystem::Instance()->GetScaleFactor();
	keyboardSize += DAVA::UIControlSystem::Instance()->GetInputOffset();

	cppTextField->GetDelegate()->OnKeyboardShown(DAVA::Rect(keyboardOrigin, keyboardSize));
}

@end

void CreateTextField(DAVA::UITextField  * tf)
{
	///[textFieldHolder->textField becomeFirstResponder];
}

void ReleaseTextField()
{
}

void OpenKeyboard()
{
}

void CloseKeyboard()
{
}

namespace DAVA 
{
    UITextFieldiPhone::UITextFieldiPhone(void  * tf)
    {
        UITextFieldHolder * textFieldHolder = [[UITextFieldHolder alloc] init: (DAVA::UITextField*)tf];
        objcClassPtr = textFieldHolder;
    }
    UITextFieldiPhone::~UITextFieldiPhone()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder removeFromSuperview];
        [textFieldHolder release];
        textFieldHolder = 0;
    }
	
    void UITextFieldiPhone::SetTextColor(const DAVA::Color &color)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        textFieldHolder->textField.textColor = [UIColor colorWithRed:color.r green:color.g blue:color.b alpha:color.a];
        
    }
    void UITextFieldiPhone::SetFontSize(float size)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        float scaledSize = size * Core::GetVirtualToPhysicalFactor();
        
        if( [[::UIScreen mainScreen] respondsToSelector:@selector(displayLinkWithTarget:selector:)])
        {
            scaledSize /= [::UIScreen mainScreen].scale;
        }
        textFieldHolder->textField.font = [UIFont systemFontOfSize:scaledSize];
    }
    
    void UITextFieldiPhone::SetTextAlign(DAVA::int32 align)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
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
	
    DAVA::int32 UITextFieldiPhone::GetTextAlign()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        
        DAVA::int32 retValue = 0;
        
        
        UIControlContentHorizontalAlignment horAligment = textFieldHolder->textField.contentHorizontalAlignment;
        UIControlContentVerticalAlignment   verAligment = textFieldHolder->textField.contentVerticalAlignment;
        
        switch (horAligment)
        {
            case UIControlContentHorizontalAlignmentLeft:
                retValue |= ALIGN_LEFT;
                break;
            case UIControlContentHorizontalAlignmentCenter:
                retValue |= ALIGN_HCENTER;
                break;
            case UIControlContentHorizontalAlignmentRight:
                retValue |= ALIGN_RIGHT;
                break;
        }
        
        switch (verAligment)
        {
            case UIControlContentVerticalAlignmentTop:
                retValue |= ALIGN_TOP;
                break;
            case UIControlContentVerticalAlignmentCenter:
                retValue |= ALIGN_VCENTER;
                break;
            case UIControlContentVerticalAlignmentBottom:
                retValue |= ALIGN_BOTTOM;
                break;
        }
        
    return retValue;
    }
    
    void UITextFieldiPhone::OpenKeyboard()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        textFieldHolder->textField.userInteractionEnabled = YES;
        [textFieldHolder->textField becomeFirstResponder];
    }
    
    void UITextFieldiPhone::CloseKeyboard()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        textFieldHolder->textField.userInteractionEnabled = NO;
        [textFieldHolder->textField resignFirstResponder];
    }
    
    void UITextFieldiPhone::ShowField()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        HelperAppDelegate* appDelegate = [[UIApplication sharedApplication] delegate];
        [[appDelegate glController].backgroundView addSubview:textFieldHolder];
        
        // Attach to "keyboard shown/keyboard hidden" notifications.
		NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
		[center addObserver:textFieldHolder selector:@selector(keyboardDidShow:)
					   name:UIKeyboardDidShowNotification object:nil];
		[center addObserver:textFieldHolder selector:@selector(keyboardWillHide:)
					   name:UIKeyboardWillHideNotification object:nil];
        
		[center addObserver:textFieldHolder selector:@selector(keyboardFrameDidChange:)
					   name:UIKeyboardDidChangeFrameNotification object:nil];

        [center addObserver:textFieldHolder selector:@selector(keyboardFrameDidChange:)
					   name:UIKeyboardDidChangeFrameNotification object:nil];
    }
    
    void UITextFieldiPhone::HideField()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;

        // Attach to "keyboard shown/keyboard hidden" notifications.
		NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
		[center removeObserver:textFieldHolder name:UIKeyboardDidShowNotification object:nil];
		[center removeObserver:textFieldHolder name:UIKeyboardWillHideNotification object:nil];
        [center removeObserver:textFieldHolder name:UIKeyboardDidChangeFrameNotification object:nil];

        [textFieldHolder removeFromSuperview];
    }
    
    void UITextFieldiPhone::UpdateRect(const Rect & rect)
    {
        float divider = GetUITextViewSizeDivider();
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        CGRect cgRect = CGRectMake((rect.x - DAVA::Core::Instance()->GetVirtualScreenXMin()) * DAVA::Core::GetVirtualToPhysicalFactor()/divider
                                   , (rect.y - DAVA::Core::Instance()->GetVirtualScreenYMin()) * DAVA::Core::GetVirtualToPhysicalFactor()/divider
                                   , rect.dx * DAVA::Core::GetVirtualToPhysicalFactor()/divider
                                   , rect.dy * DAVA::Core::GetVirtualToPhysicalFactor()/divider);
        textFieldHolder->textField.frame = cgRect;
    }
	
    void UITextFieldiPhone::SetText(std::wstring & string)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        textFieldHolder->textField.text = [[ [ NSString alloc ]  
                                            initWithBytes : (char*)string.data()   
                                            length : string.size() * sizeof(wchar_t)   
                                            encoding : CFStringConvertEncodingToNSStringEncoding ( kCFStringEncodingUTF32LE ) ] autorelease];
        
        [textFieldHolder->textField.undoManager removeAllActions];
    }
	
    void UITextFieldiPhone::GetText(std::wstring & string) const
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        
        const char * cstr = [textFieldHolder->textField.text cStringUsingEncoding:NSUTF8StringEncoding];
        DAVA::UTF8Utils::EncodeToWideString((DAVA::uint8*)cstr, strlen(cstr), string);
        
        /*
        int length = [textFieldHolder->textField.text length];
		

        string.resize(length); 
        for (int i = 0; i < length; i++) 
        {
            unichar uchar = [textFieldHolder->textField.text characterAtIndex:i];
            string[i] = (wchar_t)uchar;
        }
         */
    }

	void UITextFieldiPhone::SetIsPassword(bool isPassword)
	{
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
		[textFieldHolder setIsPassword: isPassword];
	}

	void UITextFieldiPhone::SetInputEnabled(bool value)
	{
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
		[textFieldHolder setTextInputAllowed:value];
	}

	void UITextFieldiPhone::SetAutoCapitalizationType(DAVA::int32 value)
	{
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
		textFieldHolder->textField.autocapitalizationType = [textFieldHolder convertAutoCapitalizationType:
															 (DAVA::UITextField::eAutoCapitalizationType)value];
	}

	void UITextFieldiPhone::SetAutoCorrectionType(DAVA::int32 value)
	{
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
		textFieldHolder->textField.autocorrectionType = [textFieldHolder convertAutoCorrectionType:
														 (DAVA::UITextField::eAutoCorrectionType)value];
	}

	void UITextFieldiPhone::SetSpellCheckingType(DAVA::int32 value)
	{
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_5_0
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
		textFieldHolder->textField.spellCheckingType = [textFieldHolder convertSpellCheckingType:
														 (DAVA::UITextField::eSpellCheckingType)value];
#endif
	}

	void UITextFieldiPhone::SetKeyboardAppearanceType(DAVA::int32 value)
	{
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
		textFieldHolder->textField.keyboardAppearance = [textFieldHolder convertKeyboardAppearanceType:
														(DAVA::UITextField::eKeyboardAppearanceType)value];
	}

	void UITextFieldiPhone::SetKeyboardType(DAVA::int32 value)
	{
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
		textFieldHolder->textField.keyboardType = [textFieldHolder convertKeyboardType:
														 (DAVA::UITextField::eKeyboardType)value];
	}

	void UITextFieldiPhone::SetReturnKeyType(DAVA::int32 value)
	{
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
		textFieldHolder->textField.returnKeyType = [textFieldHolder convertReturnKeyType:
												   (DAVA::UITextField::eReturnKeyType)value];
	}
	
	void UITextFieldiPhone::SetEnableReturnKeyAutomatically(bool value)
	{
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
		textFieldHolder->textField.enablesReturnKeyAutomatically = [textFieldHolder convertEnablesReturnKeyAutomatically:value];
	}

    uint32 UITextFieldiPhone::GetCursorPos()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        if (!textFieldHolder)
        {
            return 0;
        }

        ::UITextField* textField = textFieldHolder->textField;
        int pos = [textField offsetFromPosition: textField.beginningOfDocument
                                     toPosition: textField.selectedTextRange.start];
        return pos;
    }

    void UITextFieldiPhone::SetCursorPos(uint32 pos)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
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
    
    void UITextFieldiPhone::SetVisible(bool value)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        if (textFieldHolder)
        {
            ::UITextField* textField = textFieldHolder->textField;
            [textField setHidden: value == false];
        }
    }
}

#endif
