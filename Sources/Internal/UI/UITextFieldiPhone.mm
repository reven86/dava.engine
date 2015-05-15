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


#include "Base/BaseTypes.h"
#include "Core/Core.h"

#if defined(__DAVAENGINE_IPHONE__)

#include <UIKit/UIKit.h>
#include "UI/UITextField.h"
#include "UI/UITextFieldiPhone.h"
#include "Platform/TemplateiOS/UITextFieldHolder.h"
#include "Core/Core.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#include "Platform/TemplateiOS/WebViewControliOS.h"

#import "Platform/TemplateiOS/HelperAppDelegate.h"

namespace
{
    const int MOVE_TO_OFFSCREEN_STEP = 20000;
}

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
    UITextFieldiPhone::UITextFieldiPhone(DAVA::UITextField& tf):
    davaTextField(tf),
    renderToTexture(false)
    {
        HelperAppDelegate* appDelegate = [[UIApplication sharedApplication] delegate];
        BackgroundView* backgroundView = [appDelegate glController].backgroundView;
        
        UITextFieldHolder * textFieldHolder= [backgroundView CreateTextField];
        [textFieldHolder setTextField:&davaTextField];

        objcClassPtr = textFieldHolder;
        
        prevRect = tf.GetRect();
        if (renderToTexture)
        {
            UpdateNativeRect(prevRect, MOVE_TO_OFFSCREEN_STEP);
        } else
        {
            UpdateNativeRect(prevRect, 0);
        }
    }
    UITextFieldiPhone::~UITextFieldiPhone()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder setTextField:(DAVA::UITextField *)nil];
        
        HelperAppDelegate* appDelegate = [[UIApplication sharedApplication] delegate];
        BackgroundView* backgroundView = [appDelegate glController].backgroundView;

        [backgroundView ReleaseTextField:textFieldHolder];
        
        objcClassPtr = 0;
    }
	
    void UITextFieldiPhone::SetTextColor(const DAVA::Color &color)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        textFieldHolder->textField.textColor = [UIColor colorWithRed:color.r green:color.g blue:color.b alpha:color.a];
    }
    void UITextFieldiPhone::SetFontSize(float size)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        float scaledSize = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(size);
        
        scaledSize /= Core::Instance()->GetScreenScaleFactor();
        
        textFieldHolder->textField.font = [UIFont systemFontOfSize:scaledSize];
    }
    
    void UITextFieldiPhone::SetTextAlign(DAVA::int32 align)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        if (align & ALIGN_LEFT)
		{
            textFieldHolder->textField.contentHorizontalAlignment = UIControlContentHorizontalAlignmentLeft;
		}
        else if (align & ALIGN_HCENTER)
		{
            textFieldHolder->textField.contentHorizontalAlignment = UIControlContentHorizontalAlignmentCenter;
		}
        else if (align & ALIGN_RIGHT)
		{
            textFieldHolder->textField.contentHorizontalAlignment = UIControlContentHorizontalAlignmentRight;
		}
		
        if (align & ALIGN_TOP)
            textFieldHolder->textField.contentVerticalAlignment = UIControlContentVerticalAlignmentTop;
        else if (align & ALIGN_VCENTER)
            textFieldHolder->textField.contentVerticalAlignment = UIControlContentVerticalAlignmentCenter;
        else if (align & ALIGN_BOTTOM)
            textFieldHolder->textField.contentVerticalAlignment = UIControlContentVerticalAlignmentBottom;
		
		// Set natural alignment if need
		switch (textFieldHolder->textField.contentHorizontalAlignment) {
			case UIControlContentHorizontalAlignmentLeft:
				textFieldHolder->textField.textAlignment = textFieldHolder->useRtlAlign ? NSTextAlignmentNatural : NSTextAlignmentLeft;
				break;
			case UIControlContentVerticalAlignmentCenter:
				textFieldHolder->textField.textAlignment = NSTextAlignmentCenter;
				break;
			case UIControlContentHorizontalAlignmentRight:
				textFieldHolder->textField.textAlignment = textFieldHolder->useRtlAlign ? NSTextAlignmentNatural : NSTextAlignmentRight;
				break;
            default:
                break;
		}
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
                
            default: break;
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
            default: break;
        }
        
    return retValue;
    }
	
	void UITextFieldiPhone::SetTextUseRtlAlign(bool useRtlAlign)
	{
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder setUseRtlAlign:useRtlAlign];
	}
	
	bool UITextFieldiPhone::GetTextUseRtlAlign() const
	{
		UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
		return textFieldHolder->useRtlAlign == YES;
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

        DVASSERT([textFieldHolder superview] != nil);
        [textFieldHolder setHidden:NO];
        
        // Attach to "keyboard shown/keyboard hidden" notifications.
		NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
		[center addObserver:textFieldHolder selector:@selector(keyboardDidShow:)
					   name:UIKeyboardDidShowNotification object:nil];
		[center addObserver:textFieldHolder selector:@selector(keyboardWillHide:)
					   name:UIKeyboardWillHideNotification object:nil];
		[center addObserver:textFieldHolder selector:@selector(keyboardFrameDidChange:)
					   name:UIKeyboardDidChangeFrameNotification object:nil];

        auto func = [](CFNotificationCenterRef center, void *observer,
                       CFStringRef name, const void *object, CFDictionaryRef userInfo)
        {
            (static_cast<UITextFieldiPhone *>(observer))->UpdateStaticTexture();
        };
        
        CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), this,
            func,
            (__bridge CFStringRef) UIKeyboardDidHideNotification, nil,
            CFNotificationSuspensionBehaviorDeliverImmediately);
    }
    
    void UITextFieldiPhone::HideField()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder setHidden:YES];

        // Attach to "keyboard shown/keyboard hidden" notifications.
		NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
		[center removeObserver:textFieldHolder name:UIKeyboardDidShowNotification object:nil];
		[center removeObserver:textFieldHolder name:UIKeyboardWillHideNotification object:nil];
        [center removeObserver:textFieldHolder name:UIKeyboardDidChangeFrameNotification object:nil];
        
        CFNotificationCenterRemoveObserver(CFNotificationCenterGetLocalCenter(), this,
            (__bridge CFStringRef) UIKeyboardDidHideNotification, nil);
    }
    
    void UITextFieldiPhone::UpdateNativeRect(const Rect & virtualRect, int xOffset)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        
        DAVA::float32 divider = DAVA::Core::Instance()->GetScreenScaleFactor();
        DAVA::Rect physicalRect = DAVA::VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(virtualRect);
        DAVA::Vector2 physicalOffset = DAVA::VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset();
        CGRect nativeRect = CGRectMake(  (physicalRect.x + physicalOffset.x) / divider
                                       , (physicalRect.y + physicalOffset.y) / divider
                                       , physicalRect.dx / divider
                                       , physicalRect.dy / divider);

        nativeRect = CGRectIntegral(nativeRect);
        nativeRect.origin.x += xOffset;
        textFieldHolder->textField.frame = nativeRect;
    }
    
    void UITextFieldiPhone::UpdateRect(const Rect & rect)
    {
        if(renderToTexture)
        {
            UpdateNativeRect(rect, MOVE_TO_OFFSCREEN_STEP);
            if (rect.dx != prevRect.dx || rect.dy != prevRect.dy)
            {
                UpdateStaticTexture();
            }
        }
        else
        {
            UpdateNativeRect(rect, 0);
        }
        prevRect = rect;
    }
	
    void UITextFieldiPhone::SetText(const WideString & string)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        
        NSString* text = [[ [ NSString alloc ] initWithBytes : (char*)string.data()
                                                         length : string.size() * sizeof(wchar_t)
                                                       encoding : CFStringConvertEncodingToNSStringEncoding ( kCFStringEncodingUTF32LE ) ] autorelease];
        NSString* truncatedText = (NSString*)TruncateText(text, textFieldHolder->cppTextField->GetMaxLength());
        
        bool textChanged = ![textFieldHolder->textField.text isEqualToString:truncatedText];
        
        textFieldHolder->textField.text = truncatedText;
        [textFieldHolder->textField.undoManager removeAllActions];

        // Notify UITextFieldDelegate::TextFieldOnTextChanged event
        [textFieldHolder->textField sendActionsForControlEvents:UIControlEventEditingChanged];
        
        if(textChanged)
        {
            UpdateStaticTexture();
        }
    }
	
    void UITextFieldiPhone::GetText(WideString & string) const
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        
        const char * cstr = [textFieldHolder->textField.text cStringUsingEncoding:NSUTF8StringEncoding];
        DAVA::UTF8Utils::EncodeToWideString((DAVA::uint8*)cstr, strlen(cstr), string);
    }

	void UITextFieldiPhone::SetIsPassword(bool isPassword)
	{
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
		[textFieldHolder setIsPassword: isPassword];
        UpdateStaticTexture();
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
        int32 pos = static_cast<int32>([textField offsetFromPosition: textField.beginningOfDocument
                                     toPosition: textField.selectedTextRange.start]);
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
            pos = static_cast<uint32>(textLength - 1);
        }

        UITextPosition *start = [textField positionFromPosition:[textField beginningOfDocument] offset:pos];
        UITextPosition *end = [textField positionFromPosition:start offset:0];
        [textField setSelectedTextRange:[textField textRangeFromPosition:start toPosition:end]];
    }
    
    void UITextFieldiPhone::SetVisible(bool value)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder setHidden: value == false];
    }

    void UITextFieldiPhone::SetMaxLength(int maxLength)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        if (textFieldHolder)
        {
            ::UITextField* textField = textFieldHolder->textField;
            [textField setText: (NSString*)TruncateText([textField text], maxLength)];
        }
    }
    
    void UITextFieldiPhone::UpdateStaticTexture()
    {
        if (renderToTexture)
        {
            UITextFieldHolder * textFieldHolder = static_cast<UITextFieldHolder*>(objcClassPtr);
            DVASSERT(textFieldHolder);
            ::UITextField* textField = textFieldHolder->textField;
            DVASSERT(textField);
            void* imgPtr = DAVA::WebViewControl::RenderIOSUIViewToImage(textField);
            ::UIImage* image = static_cast<::UIImage*>(imgPtr);
            if (nullptr != image) // can't render to empty rect so skip
            {
                // set backgroud image into davaTextField control
                WebViewControl::SetImageAsSpriteToControl(image, davaTextField);
            }
        }
        else
        {
            // set null background
            davaTextField.GetBackground()->SetSprite(nullptr, 0);
        }
    }
    
    void UITextFieldiPhone::SetRenderToTexture(bool value)
    {
        renderToTexture = value;
        UpdateStaticTexture();
    }
    
    bool UITextFieldiPhone::IsRenderToTexture() const
    {
        return renderToTexture;
    }

    void* UITextFieldiPhone::TruncateText(void* text, int maxLength)
    {
        if (maxLength >= 0)
        {
            NSString* textString = (NSString*)text;
            NSUInteger textLimit = MIN([textString length], (NSUInteger)maxLength);
            return [textString substringToIndex: textLimit];
        }

        return text;
    }
}

#endif
