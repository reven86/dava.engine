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

#include "UI/UISystemKeyboardImpl.h"
#include "UI/UISystemKeyboard.h"
#include "HelperAppDelegate.h"
#import <Foundation/Foundation.h>


@interface UISystemKeyboardNativeImpl : NSObject
{
@private
    DAVA::UISystemKeyboardImpl * keyboard;
}
- (id) init : (DAVA::UISystemKeyboardImpl  *) kb;
- (void) Subscribe;
- (void) Unsubscribe;
@end

namespace DAVA
{
UISystemKeyboardImpl::UISystemKeyboardImpl( UISystemKeyboard *kb )
{
    keyboard = kb;
    UISystemKeyboardNativeImpl * impl = [[UISystemKeyboardNativeImpl alloc] init: (DAVA::UISystemKeyboardImpl*) this];
    implPointer = impl;
    [impl Subscribe];
}
    
UISystemKeyboardImpl::~UISystemKeyboardImpl()
{
    UISystemKeyboardNativeImpl * impl = (UISystemKeyboardNativeImpl*)implPointer;
    [impl Unsubscribe];
    [impl release];
    implPointer = NULL;
}

void UISystemKeyboardImpl::SendWillShowNotification( const Rect &kbRect )
{
    keyboard->SendWillShowNotification( kbRect );
}

void UISystemKeyboardImpl::SendDidShowNotification( const Rect &kbRect )
{
    keyboard->SendDidShowNotification( kbRect );
}

void UISystemKeyboardImpl::SendWillHideNotification()
{
    keyboard->SendWillHideNotification();
}

void UISystemKeyboardImpl::SendDidHideNotification()
{
    keyboard->SendDidHideNotification();
}

};

@implementation UISystemKeyboardNativeImpl

- (id) init : (DAVA::UISystemKeyboardImpl  *) kb
{
    if (self = [super init])
	{
        keyboard = kb;
    }
    return self;
}

-(void)Subscribe
{
     //Attach to "keyboard shown/keyboard hidden" notifications.
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(keyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];
    [center addObserver:self selector:@selector(keyboardDidShow:)  name:UIKeyboardDidShowNotification object:nil];
    [center addObserver:self selector:@selector(keyboardWillHide:) name:UIKeyboardWillHideNotification object:nil];
    [center addObserver:self selector:@selector(keyboardDidHide:)  name:UIKeyboardDidHideNotification object:nil];
}

-(void)Unsubscribe
{
    // Attach to "keyboard shown/keyboard hidden" notifications.
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self name:UIKeyboardWillShowNotification object:nil];
    [center removeObserver:self name:UIKeyboardDidShowNotification object:nil];
    [center removeObserver:self name:UIKeyboardWillHideNotification object:nil];
    [center removeObserver:self name:UIKeyboardDidHideNotification object:nil];
}

- (void) ShowKeyboard : (UITextField *) textField
{
    textField.userInteractionEnabled = YES;
    [textField becomeFirstResponder];
}

- (void) HideKeyboard : (UITextField *) textField
{
    textField.userInteractionEnabled = NO;
    [textField resignFirstResponder];
}

-(void)keyboardWillShow:(NSNotification *)notification
{
    NSDictionary* userInfo = notification.userInfo;
    CGRect keyboardFrame = [[notification.userInfo objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
    
    // Recalculate to virtual coordinates.
	DAVA::Vector2 keyboardOrigin(keyboardFrame.origin.x, keyboardFrame.origin.y);
	keyboardOrigin *= DAVA::UIControlSystem::Instance()->GetScaleFactor();
	keyboardOrigin += DAVA::UIControlSystem::Instance()->GetInputOffset();
	
	DAVA::Vector2 keyboardSize(keyboardFrame.size.width, keyboardFrame.size.height);
	keyboardSize *= DAVA::UIControlSystem::Instance()->GetScaleFactor();
	keyboardSize += DAVA::UIControlSystem::Instance()->GetInputOffset();
    
    keyboard->SendWillShowNotification( DAVA::Rect(keyboardOrigin, keyboardSize) );
}

-(void)keyboardDidShow:(NSNotification *)notification
{
    NSDictionary* userInfo = notification.userInfo;
    CGRect keyboardFrame = [[notification.userInfo objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
    
    HelperAppDelegate* appDelegate = [[UIApplication sharedApplication] delegate];
    
    keyboardFrame = [appDelegate.window convertRect:(CGRect)keyboardFrame toView:(UIView *)[appDelegate glController].backgroundView];
    // Recalculate to virtual coordinates.
	DAVA::Vector2 keyboardOrigin(keyboardFrame.origin.x, keyboardFrame.origin.y);
	keyboardOrigin *= DAVA::UIControlSystem::Instance()->GetScaleFactor();
	keyboardOrigin += DAVA::UIControlSystem::Instance()->GetInputOffset();
	
	DAVA::Vector2 keyboardSize(keyboardFrame.size.width, keyboardFrame.size.height);
	keyboardSize *= DAVA::UIControlSystem::Instance()->GetScaleFactor();
	keyboardSize += DAVA::UIControlSystem::Instance()->GetInputOffset();
    
    keyboard->SendDidShowNotification( DAVA::Rect(keyboardOrigin, keyboardSize) );
}

-(void)keyboardWillHide:(NSNotification *)notification
{
    keyboard->SendWillHideNotification();
}

-(void)keyboardDidHide:(NSNotification *)notification
{
    keyboard->SendDidHideNotification();
}

@end
