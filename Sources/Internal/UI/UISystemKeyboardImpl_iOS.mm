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
//- (void) ShowKeyboard : (UITextField *) textField;
//- (void) HideKeyboard : (UITextField *) textField;
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

/*
void UISystemKeyboardImpl::Show( DAVA::UITextField * davaTextField )
{
    ::UITextField * textField = NULL;
    UIKeyboard_iOSImpl * impl = (UIKeyboard_iOSImpl*)implPointer;
    [ impl ShowKeyboard: (::UITextField*) textField ];
    
}
    
void UISystemKeyboardImpl::Hide( DAVA::UITextField * davaTextField )
{
    ::UITextField * textField = NULL;
    UIKeyboard_iOSImpl * impl = (UIKeyboard_iOSImpl*)implPointer;
    [ impl HideKeyboard: (::UITextField*) textField ];
}
 */   
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
    [center addObserver:self selector:@selector(keyboardFrameWillChange:) name:UIKeyboardWillChangeFrameNotification object:nil];
    [center addObserver:self selector:@selector(keyboardFrameDidChange:) name:UIKeyboardDidChangeFrameNotification object:nil];
}

-(void)Unsubscribe
{
    // Attach to "keyboard shown/keyboard hidden" notifications.
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self name:UIKeyboardWillShowNotification object:nil];
    [center removeObserver:self name:UIKeyboardDidShowNotification object:nil];
    [center removeObserver:self name:UIKeyboardWillHideNotification object:nil];
    [center removeObserver:self name:UIKeyboardDidHideNotification object:nil];
    [center removeObserver:self name:UIKeyboardWillChangeFrameNotification object:nil];
    [center removeObserver:self name:UIKeyboardDidChangeFrameNotification object:nil];
    
    //[center removeObserver:self];
    //[self removeFromSuperview];
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

-(void)keyboardFrameWillChange:(NSNotification *)notification
{}

-(void)keyboardFrameDidChange:(NSNotification *)notification
{}

@end
