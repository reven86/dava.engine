/*==================================================================================
 Copyright (c) 2008, DAVA Consulting, LLC
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the DAVA Consulting, LLC nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Revision History:
 * Created by Alexey 'Hottych' Prosin
 =====================================================================================*/

#include "BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include <UIKit/UIKit.h>
#include "UI/UITextField.h"
#include "UI/UITextFieldiPhone.h"
#include "Core/Core.h"
#include "UITextFieldHolder.h"

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

@implementation UITextField (DisableEditMenu)

-(BOOL)canPerformAction:(SEL)action withSender:(id)sender
{    
    [UIMenuController sharedMenuController].menuVisible = NO;
    return NO;
}

@end

@implementation UITextFieldHolder

- (void)setOrientationAngle:(float) angle
{
    self.transform = CGAffineTransformMakeRotation(DAVA::DegToRad(angle));
}

- (id) init : (DAVA::UITextField  *) tf
{
	if (self = [super init])
	{
        float divider = GetUITextViewSizeDivider();
        
        float orientationAngle = 0.f;
        UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
        if(orientation == UIInterfaceOrientationLandscapeLeft)
            orientationAngle = -90.f;
        if(orientation == UIInterfaceOrientationLandscapeRight)
            orientationAngle = 90.f;
        
        [self setOrientationAngle:orientationAngle];
        
		self.bounds = CGRectMake(0.0f, 0.0f, DAVA::Core::Instance()->GetPhysicalScreenWidth()/divider, DAVA::Core::Instance()->GetPhysicalScreenHeight()/divider);
		self.center = CGPointMake(DAVA::Core::Instance()->GetPhysicalScreenHeight()/2/divider, DAVA::Core::Instance()->GetPhysicalScreenWidth()/2/divider);	
		self.userInteractionEnabled = FALSE;
        
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
		
        textField.textAlignment = UITextAlignmentCenter;
        
		[self addSubview:textField];
	}
	return self;
}


- (void) dealloc
{
	[textField release];
	textField = 0;
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
			int length = [string length];
			
			DAVA::WideString repString;
			repString.resize(length); 
			for (int i = 0; i < length; i++) 
			{
				unichar uchar = [string characterAtIndex:i];
				repString[i] = (wchar_t)uchar;
			}
			return cppTextField->GetDelegate()->TextFieldKeyPressed(cppTextField, range.location, range.length, repString);
		}
	}
	return TRUE;
}

- (void)textFieldDidBeginEditing:(UITextField *)textField_
{
    
    [textField_ selectAll:textField_];
    [UIMenuController sharedMenuController].menuVisible = NO;
    
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
	
    void UITextFieldiPhone::SetFontColor(float r, float g, float b, float a)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        textFieldHolder->textField.textColor = [UIColor colorWithRed:r green:g blue:b alpha:a];
    }
    void UITextFieldiPhone::SetFontSize(float size)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        textFieldHolder->textField.font = [UIFont systemFontOfSize:size * Core::GetVirtualToPhysicalFactor()];
    }
	
    void UITextFieldiPhone::SetReturnKey(int32 returnType)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        switch (returnType) 
        {
            case UITextField::RETURN_KEY_RETURN:
            {
                textFieldHolder->textField.returnKeyType = UIReturnKeyRoute;
            }
                break;
            case UITextField::RETURN_KEY_DONE:
            {
                textFieldHolder->textField.returnKeyType = UIReturnKeyDone;
            }
                break;
        }
        
    }
    
    
    void UITextFieldiPhone::OpenKeyboard()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder->textField becomeFirstResponder];
    }
    
    void UITextFieldiPhone::CloseKeyboard()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder->textField resignFirstResponder];
    }
    
    void UITextFieldiPhone::ShowField()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [[[UIApplication sharedApplication] keyWindow] addSubview: textFieldHolder];
    }
    
    void UITextFieldiPhone::HideField()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
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
    }
	
    void UITextFieldiPhone::GetText(std::wstring & string)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        int length = [textFieldHolder->textField.text length];
		
        string.resize(length); 
        for (int i = 0; i < length; i++) 
        {
            unichar uchar = [textFieldHolder->textField.text characterAtIndex:i];
            string[i] = (wchar_t)uchar;
        }
    }
    
    void UITextFieldiPhone::SetOrientationAngle(float32 angle)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder setOrientationAngle: angle];
    }
}

#endif
