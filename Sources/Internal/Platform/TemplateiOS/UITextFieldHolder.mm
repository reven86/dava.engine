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
        DAVA::float32 divider = [HelperAppDelegate GetScale];
        
        DAVA::Size2i physicalScreenSize = DAVA::VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize();
        
        self.bounds = CGRectMake(0.0f, 0.0f, physicalScreenSize.dx/divider, physicalScreenSize.dy/divider);
		self.center = CGPointMake(physicalScreenSize.dx/2.f/divider, physicalScreenSize.dy/2.f/divider);
        
		self.userInteractionEnabled = TRUE;
		textInputAllowed = YES;
        useRtlAlign = NO;

        textField = [[UITextField alloc] initWithFrame: CGRectMake(0.f, 0.f, 0.f, 0.f)];
		textField.delegate = self;
		
		[self setupTraits];
        
        textField.userInteractionEnabled = NO;

        cachedText = [[NSString alloc] initWithString:textField.text];
        
        [textField addTarget: self
                      action: @selector(eventEditingChanged:)
            forControlEvents: UIControlEventEditingChanged];

		// Done!
		[self addSubview:textField];
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
        
        textField.frame = nativeRect;

    }
    else
    {
        textField.frame = CGRectMake(0.f, 0.f, 0.f, 0.f);
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

-(id)hitTest:(CGPoint)point withEvent:(UIEvent *)event
{
    id hitView = [super hitTest:point withEvent:event];
    if (hitView == self) return nil;
    else return hitView;
}

- (void) dealloc
{
    [cachedText release];
    cachedText = nil;
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
	if (cppTextField && (cppTextField->GetDelegate() != 0))
    {
        // Check length limits.
        BOOL needIgnoreDelegateResult = FALSE;
        int maxLength = cppTextField->GetMaxLength();
        if (maxLength >= 0)
        {
            NSString* newString = [textField.text stringByReplacingCharactersInRange:range withString:string]; // Get string after changing
            NSUInteger newLength = [newString lengthOfBytesUsingEncoding:NSUTF32StringEncoding] / 4; // Length in UTF32 charactres
            if (newLength > (NSUInteger)maxLength)
            {
                NSUInteger charsToInsert = 0;
                if (range.length == 0)
                {
                    NSUInteger curLength = [textField.text lengthOfBytesUsingEncoding:NSUTF32StringEncoding] / 4; // Length in UTF32 charactres
                    // Inserting without replace.
                    charsToInsert = (NSUInteger)maxLength - curLength;
                }
                else
                {
                    // Inserting with replace.
                    charsToInsert = range.length;
                }
                
                // Convert NSString to UTF32 bytes array with length of charsToInsert*4 and
                // back for decrease string length in UTF32 code points
                NSUInteger byteCount = charsToInsert * 4; // 4 bytes per utf32 character
                char buffer[byteCount];
                NSUInteger usedBufferCount;
                [string getBytes:buffer maxLength:byteCount usedLength:&usedBufferCount encoding:NSUTF32StringEncoding options:0 range:NSMakeRange(0, string.length) remainingRange:NULL];
                DVASSERT(string && "Error on convert utf32 to NSString");
                
                needIgnoreDelegateResult = TRUE;
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

	return TRUE;
}

- (void)eventEditingChanged:(UITextField *)sender
{
    if (sender == textField && cppTextField && cppTextField->GetDelegate()
        && ![cachedText isEqualToString:textField.text])
    {
        DAVA::WideString oldString;
        const char * cstr = [cachedText cStringUsingEncoding:NSUTF8StringEncoding];
        DAVA::UTF8Utils::EncodeToWideString((DAVA::uint8*)cstr, (DAVA::int32)strlen(cstr), oldString);
        
        [cachedText release];
        cachedText = [[NSString alloc] initWithString:textField.text];
        
        DAVA::WideString newString;
        cstr = [cachedText cStringUsingEncoding:NSUTF8StringEncoding];
        DAVA::UTF8Utils::EncodeToWideString((DAVA::uint8*)cstr, (DAVA::int32)strlen(cstr), newString);
        
        cppTextField->GetDelegate()->TextFieldOnTextChanged(cppTextField, newString, oldString);
    }
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

- (void)setUseRtlAlign:(bool)value
{
    useRtlAlign = (value == true);
    
    // Set natural alignment if need
    switch (textField.contentHorizontalAlignment)
    {
        case UIControlContentHorizontalAlignmentLeft:
            textField.textAlignment = useRtlAlign ? NSTextAlignmentNatural : NSTextAlignmentLeft;
            break;
        case UIControlContentHorizontalAlignmentRight:
            textField.textAlignment = useRtlAlign ? NSTextAlignmentNatural : NSTextAlignmentRight;
            break;
            
        default: break;
    }
}

- (void) setupTraits
{
	if (!cppTextField || !textField)
	{
		return;
	}

	textField.autocapitalizationType = [self convertAutoCapitalizationType: (DAVA::UITextField::eAutoCapitalizationType)cppTextField->GetAutoCapitalizationType()];
	textField.autocorrectionType = [self convertAutoCorrectionType: (DAVA::UITextField::eAutoCorrectionType)cppTextField->GetAutoCorrectionType()];
	
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_5_0
	textField.spellCheckingType = [self convertSpellCheckingType: (DAVA::UITextField::eSpellCheckingType)cppTextField->GetSpellCheckingType()];
#endif
	textField.enablesReturnKeyAutomatically = [self convertEnablesReturnKeyAutomatically: cppTextField->IsEnableReturnKeyAutomatically()];
	textField.keyboardAppearance = [self convertKeyboardAppearanceType: (DAVA::UITextField::eKeyboardAppearanceType)cppTextField->GetKeyboardAppearanceType()];
	textField.keyboardType = [self convertKeyboardType: (DAVA::UITextField::eKeyboardType)cppTextField->GetKeyboardType()];
	textField.returnKeyType = [self convertReturnKeyType: (DAVA::UITextField::eReturnKeyType)cppTextField->GetReturnKeyType()];
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
	CGRect ownFrame = [textField.window convertRect:self.frame fromView:textField.superview];

	// calculate the area of own frame that is covered by keyboard
	CGRect keyboardFrame = CGRectIntersection(ownFrame, lastKeyboardFrame);

	// now this might be rotated, so convert it back
	keyboardFrame = [textField.window convertRect:keyboardFrame toView:textField.superview];

	// Recalculate to virtual coordinates.
	DAVA::Vector2 keyboardOrigin(keyboardFrame.origin.x, keyboardFrame.origin.y);
    keyboardOrigin = DAVA::VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual(keyboardOrigin);
	
    DAVA::Vector2 keyboardSize(keyboardFrame.size.width, keyboardFrame.size.height);
    keyboardSize = DAVA::VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual(keyboardSize);

	cppTextField->GetDelegate()->OnKeyboardShown(DAVA::Rect(keyboardOrigin, keyboardSize));
}


@end

#endif //#if defined(__DAVAENGINE_IPHONE__)
