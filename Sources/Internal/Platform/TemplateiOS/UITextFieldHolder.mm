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


#include "UITextFieldHolder.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "UI/UITextFieldiPhone.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#import <Platform/TemplateiOS/HelperAppDelegate.h>

@implementation UITextFieldHolder

- (id) init
{
	if (self = [super init])
	{
        DAVA::float32 divider = DAVA::Core::Instance()->GetScreenScaleFactor();
        
        DAVA::Size2i physicalScreenSize = DAVA::VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize();
        
        self.bounds = CGRectMake(0.0f, 0.0f, physicalScreenSize.dx/divider, physicalScreenSize.dy/divider);
		self.center = CGPointMake(physicalScreenSize.dx/2.f/divider, physicalScreenSize.dy/2.f/divider);
        
		self.userInteractionEnabled = TRUE;
		textInputAllowed = YES;
        useRtlAlign = NO;

        textCtrl = [[UITextField alloc] initWithFrame: CGRectMake(0.f, 0.f, 0.f, 0.f)];
        [textCtrl setValue:self forKey:@"delegate"];
		
		[self setupTraits];
        
        textCtrl.userInteractionEnabled = NO;

        cachedText = [[NSString alloc] initWithString: [textCtrl valueForKey:@"text"]];
        
        if([textCtrl respondsToSelector:@selector(addTarget:action:forControlEvents:)]) {
            [(id)textCtrl addTarget: self
                          action: @selector(eventEditingChanged:)
                forControlEvents: UIControlEventEditingChanged];
        }
        
        textField = nullptr;

		// Done!
		[self addSubview:textCtrl];
	}
	return self;
}

- (void) setTextField:(DAVA::UITextField *) tf
{
    cppTextField = tf;
    if(tf)
    {
        DAVA::Rect physicalRect = DAVA::VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(tf->GetRect());
        DAVA::Vector2 physicalOffset = DAVA::VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset();
        CGRect nativeRect = CGRectMake(  (physicalRect.x + physicalOffset.x)
                                       , (physicalRect.y + physicalOffset.y)
                                       , physicalRect.dx
                                       , physicalRect.dy);
        
        textCtrl.frame = nativeRect;

    }
    else
    {
        textCtrl.frame = CGRectMake(0.f, 0.f, 0.f, 0.f);
    }
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

// next method(hitTest) fix sometime field editing(do not remove it)
-(id)hitTest:(CGPoint)point withEvent:(UIEvent *)event
{
    id hitView = [super hitTest:point withEvent:event];
    if (hitView == self) return nil;
    else return hitView;
}

-(void)textViewDidBeginEditing:(UITextView *)textView
{
    if (cppTextField)
    {
        if (DAVA::UIControlSystem::Instance()->GetFocusedControl() != cppTextField)
        {
            DAVA::UIControlSystem::Instance()->SetFocusedControl(cppTextField, false);
        }
    }
}

- (void) dealloc
{
    [cachedText release];
    cachedText = nullptr;
	[textCtrl release];
    textCtrl = nullptr;
    [textField release];
    textField = nullptr;
    
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

- (BOOL)textField:(UITextField *)textField_ shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
	if (cppTextField && (cppTextField->GetDelegate() != 0))
    {
        // Check length limits.
        BOOL needIgnoreDelegateResult = FALSE;
        int maxLength = cppTextField->GetMaxLength();
        if (maxLength >= 0)
        {
            NSString* newString = [[textCtrl valueForKey:@"text"] stringByReplacingCharactersInRange:range withString:string]; // Get string after changing
            NSInteger newLength = [newString length]; // Length in UTF32 charactres
            if (newLength > maxLength)
            {
                NSUInteger charsToInsert = 0;
                if (range.length == 0)
                {
                    // charactres count independent from encoding and bytes per each charracter
                    NSUInteger curLength = [[textCtrl valueForKey:@"text"] length];
                    // Inserting without replace.
                    if (maxLength >= (NSInteger)curLength)
                    {
                        charsToInsert = maxLength - (NSInteger)curLength;
                    }
                    else
                    {
                        charsToInsert = 0;
                    }
                }
                else
                {
                    // Inserting with replace.
                    charsToInsert = range.length;
                }
                
                
                if (0 < charsToInsert)
                {
                    // Convert NSString to UTF32 bytes array with length of charsToInsert*4 and
                    // back for decrease string length in UTF32 code points
                    NSUInteger byteCount = charsToInsert * 4; // 4 bytes per utf32 character
                    char buffer[byteCount];
                    NSUInteger usedBufferCount;
                    [string getBytes:buffer maxLength:byteCount usedLength:&usedBufferCount encoding:NSUTF32StringEncoding options:0 range:NSMakeRange(0, string.length) remainingRange:NULL];
                    string = [[NSString alloc] initWithBytes:buffer length:usedBufferCount encoding:NSUTF32LittleEndianStringEncoding];
                    DVASSERT(string && "Error on convert utf32 to NSString");
                    
                    NSString* currentText =[textCtrl valueForKey:@"text"];
                    NSString* newText = [currentText stringByReplacingCharactersInRange:range withString:string];
                    [textCtrl setValue:newText forKey:@"text"];
                    needIgnoreDelegateResult = TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
        }

        // Length check OK, continue with the delegate.
        DAVA::WideString repString;
        const char * cstr = [string cStringUsingEncoding:NSUTF8StringEncoding];
        if(cstr) //cause strlen(nullptr) will crash
        {
            DAVA::UTF8Utils::EncodeToWideString((DAVA::uint8*)cstr, (DAVA::int32)strlen(cstr), repString);
        }

        BOOL delegateResult = cppTextField->GetDelegate()->TextFieldKeyPressed(cppTextField, (DAVA::int32)range.location, (DAVA::int32)range.length, repString);
        return needIgnoreDelegateResult ? FALSE : delegateResult;
	}

	return YES;
}

- (BOOL)textView:(UITextView*)textView_ shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)string
{
    BOOL result = [self textField:(UITextField*)textView_ shouldChangeCharactersInRange:range replacementString:string];
    return result;
}

- (void)eventEditingChanged:(UIView *)sender
{
    NSString* fieldText = [textCtrl valueForKey:@"text"];
    
    if (sender == textCtrl
        && cppTextField
        && cppTextField->GetDelegate()
        && ![cachedText isEqualToString:fieldText])
    {
        DAVA::WideString oldString;
        const char * cstr = [cachedText cStringUsingEncoding:NSUTF8StringEncoding];
        DAVA::UTF8Utils::EncodeToWideString((DAVA::uint8*)cstr, (DAVA::int32)strlen(cstr), oldString);
        
        
        // Workaround: Additional check on the maximum length for cases
        // where the system event shouldChangeCharactersInRange does not work for Asian keyboards
        int maxLength = cppTextField->GetMaxLength();
        if(maxLength > 0 && (int)fieldText.length > maxLength)
        {
            fieldText = [fieldText substringToIndex:maxLength];
            if ([textCtrl class] == [::UITextField class])
            {
                auto textFieldPtr = (::UITextField*)textCtrl;
                auto selection = [textFieldPtr selectedTextRange];
                [textFieldPtr setText:fieldText];
                [textFieldPtr setSelectedTextRange:selection];
            } else {
                auto textViewPtr = (::UITextView*)textCtrl;
                auto selection = [textViewPtr selectedTextRange];
                [textViewPtr setText:fieldText];
                [textViewPtr setSelectedTextRange:selection];
            }
        }
        // End workaround
        
        [cachedText release];
        cachedText = [[NSString alloc] initWithString:fieldText];
        
        DAVA::WideString newString;
        cstr = [cachedText cStringUsingEncoding:NSUTF8StringEncoding];
        DAVA::UTF8Utils::EncodeToWideString((DAVA::uint8*)cstr, (DAVA::int32)strlen(cstr), newString);
        
        cppTextField->GetDelegate()->TextFieldOnTextChanged(cppTextField, newString, oldString);
    }
}

- (void)textViewDidChange:(UITextView*)textView
{
    [self eventEditingChanged:textView];
}

- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField
{
	return textInputAllowed;
}

- (BOOL)textViewShouldBeginEditing:(UITextView *)textView
{
    return textInputAllowed;
}

- (void)dropCachedText
{
    [cachedText release];
    cachedText = [[NSString alloc] initWithString:[textCtrl valueForKey:@"text"]];
}

- (void)setIsPassword:(bool)isPassword
{
    if ([textCtrl respondsToSelector:@selector(setSecureTextEntry:)]) {
        [(id)textCtrl setSecureTextEntry:isPassword ? YES: NO];
    }
}

- (void)setTextInputAllowed:(bool)value
{
	textInputAllowed = (value == true);
}

- (void)setUseRtlAlign:(bool)value
{
    useRtlAlign = (value == true);
    
    if ([textCtrl class] == [::UITextField class])
    {
        ::UITextField* textFieldPtr = (::UITextField*)textCtrl;
        // Set natural alignment if need
        switch (textFieldPtr.contentHorizontalAlignment)
        {
            case UIControlContentHorizontalAlignmentLeft:
                textFieldPtr.textAlignment = useRtlAlign ? NSTextAlignmentNatural : NSTextAlignmentLeft;
                break;
            case UIControlContentHorizontalAlignmentRight:
                textFieldPtr.textAlignment = useRtlAlign ? NSTextAlignmentNatural : NSTextAlignmentRight;
                break;
                
            default: break;
        }
    } else {
        DAVA::Logger::Error("UITextField::setUseRtlAlign not work in multiline");
    }
}

- (void) setupTraits
{
	if (!cppTextField || !textCtrl)
	{
		return;
	}
    
    if ([textCtrl class] == [::UITextView class])
    {
        ::UITextView* textView = (::UITextView*)textCtrl;
        
        textView.autocapitalizationType = [self convertAutoCapitalizationType: (DAVA::UITextField::eAutoCapitalizationType)cppTextField->GetAutoCapitalizationType()];
        textView.autocorrectionType = [self convertAutoCorrectionType: (DAVA::UITextField::eAutoCorrectionType)cppTextField->GetAutoCorrectionType()];
        
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_5_0
        textView.spellCheckingType = [self convertSpellCheckingType: (DAVA::UITextField::eSpellCheckingType)cppTextField->GetSpellCheckingType()];
#endif
        textView.enablesReturnKeyAutomatically = [self convertEnablesReturnKeyAutomatically: cppTextField->IsEnableReturnKeyAutomatically()];
        textView.keyboardAppearance = [self convertKeyboardAppearanceType: (DAVA::UITextField::eKeyboardAppearanceType)cppTextField->GetKeyboardAppearanceType()];
        textView.keyboardType = [self convertKeyboardType: (DAVA::UITextField::eKeyboardType)cppTextField->GetKeyboardType()];
        textView.returnKeyType = [self convertReturnKeyType: (DAVA::UITextField::eReturnKeyType)cppTextField->GetReturnKeyType()];
    } else
    {
        ::UITextField* textFieldPtr = (::UITextField*)textCtrl;
        
        textFieldPtr.autocapitalizationType = [self convertAutoCapitalizationType: (DAVA::UITextField::eAutoCapitalizationType)cppTextField->GetAutoCapitalizationType()];
        textFieldPtr.autocorrectionType = [self convertAutoCorrectionType: (DAVA::UITextField::eAutoCorrectionType)cppTextField->GetAutoCorrectionType()];
        
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_5_0
        textFieldPtr.spellCheckingType = [self convertSpellCheckingType: (DAVA::UITextField::eSpellCheckingType)cppTextField->GetSpellCheckingType()];
#endif
        textFieldPtr.enablesReturnKeyAutomatically = [self convertEnablesReturnKeyAutomatically: cppTextField->IsEnableReturnKeyAutomatically()];
        textFieldPtr.keyboardAppearance = [self convertKeyboardAppearanceType: (DAVA::UITextField::eKeyboardAppearanceType)cppTextField->GetKeyboardAppearanceType()];
        textFieldPtr.keyboardType = [self convertKeyboardType: (DAVA::UITextField::eKeyboardType)cppTextField->GetKeyboardType()];
        textFieldPtr.returnKeyType = [self convertReturnKeyType: (DAVA::UITextField::eReturnKeyType)cppTextField->GetReturnKeyType()];
    }
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
	CGRect ownFrame = [textCtrl.window convertRect:self.frame fromView:textCtrl.superview];

	// calculate the area of own frame that is covered by keyboard
	CGRect keyboardFrame = CGRectIntersection(ownFrame, lastKeyboardFrame);

	// now this might be rotated, so convert it back
	keyboardFrame = [textCtrl.window convertRect:keyboardFrame toView:textCtrl.superview];

	// Recalculate to virtual coordinates.
	DAVA::Vector2 keyboardOrigin(keyboardFrame.origin.x, keyboardFrame.origin.y);
    keyboardOrigin = DAVA::VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual(keyboardOrigin);
	
    DAVA::Vector2 keyboardSize(keyboardFrame.size.width, keyboardFrame.size.height);
    keyboardSize = DAVA::VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual(keyboardSize);

	cppTextField->GetDelegate()->OnKeyboardShown(DAVA::Rect(keyboardOrigin, keyboardSize));
}

@end

#endif //#if defined(__DAVAENGINE_IPHONE__)
