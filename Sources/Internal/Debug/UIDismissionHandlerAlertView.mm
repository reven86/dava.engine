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


#import "UIDismissionHandlerAlertView.h"
#import <UIKit/UIKit.h>

@interface UIDismissionHandlerAlertView() <UIAlertViewDelegate>

@property (retain, nonatomic) UIDismissionHandlerAlertView *alertReference;
@property (copy) AlertDismissedHandler activeDismissHandler;
@property (retain, nonatomic) UIAlertView *activeAlert;

@end

@implementation UIDismissionHandlerAlertView

- (id)initWithTitle:(NSString *)aTitle message:(NSString *)aMessage cancelButtonTitle:(NSString *)aCancelTitle otherButtonTitles:(NSString *)otherTitles,...
{
    self = [super init];
    if (self)
	{
        UIAlertView *alert = [[UIAlertView alloc] initWithTitle:aTitle message:aMessage delegate:self cancelButtonTitle:aCancelTitle otherButtonTitles:nil];
        if (otherTitles != nil)
		{
            [alert addButtonWithTitle:otherTitles];
            va_list args;
            va_start(args, otherTitles);
            NSString * title = nil;
            while((title = va_arg(args,NSString*)))
			{
                [alert addButtonWithTitle:title];
            }
			
            va_end(args);
        }
		
        self.activeAlert = alert;
    }
	
    return self;
}

- (void)showWithDismissHandler:(AlertDismissedHandler)handler
{
    self.activeDismissHandler = handler;
    self.alertReference = self;
    [self.activeAlert show];
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (self.activeDismissHandler)
	{
        self.activeDismissHandler(buttonIndex,buttonIndex == alertView.cancelButtonIndex);
    }
	
	self.activeAlert = nil;
    self.alertReference = nil;
}

-(NSString*) getMessage
{
	if (self.activeAlert)
	{
		return self.activeAlert.message;
	}
	
	return nil;
}

-(void) dismiss:(BOOL)isAnimated
{
	if (self.activeAlert)
	{
		[self.activeAlert dismissWithClickedButtonIndex:0 animated:isAnimated];
		self.activeAlert = nil;
		self.alertReference = nil;
	}
}

@end
