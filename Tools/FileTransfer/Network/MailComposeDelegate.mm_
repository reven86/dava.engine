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
#import "MailComposeDelegate.h"

#import <MessageUI/MFMailComposeViewController.h>

@implementation MailComposeDelegate


-(void)mailComposeController:(MFMailComposeViewController*)controller
		 didFinishWithResult:(MFMailComposeResult)result
					   error:(NSError*)error

{
	[controller dismissModalViewControllerAnimated:YES];
	[self release];
}

-(id) initWithController:(UIViewController*) _controller
				   email:(NSString*) _msgEmail
				 subject:(NSString*) _msgSubj
				 message:(NSString*) _msgBody
{
	if ( self = [super init] )
	{
		controller = _controller;
		msgEmail = _msgEmail;
		msgSubj = _msgSubj;
		msgBody =_msgBody;
	}
	return self;
}

-(BOOL) show
{
	if ([MFMailComposeViewController canSendMail])
	{
		[self retain];
		MFMailComposeViewController *mailViewController = [[MFMailComposeViewController alloc] init];
		mailViewController.mailComposeDelegate = self;
		
		NSArray *toRecipients = [NSArray arrayWithObjects:msgEmail, nil];
		[mailViewController setToRecipients:toRecipients];
		[mailViewController setSubject:msgSubj];
		[mailViewController setMessageBody:msgBody isHTML:NO];
		
		[controller presentModalViewController:mailViewController animated:YES];
		[mailViewController release];
		return true;
	}
	else
	{
		NSString* mailtoString = [NSString stringWithFormat:@"mailto:?to=%@&subject=%@&body=%@",
								  msgEmail, msgSubj, msgBody];
		NSString* encodedString = [mailtoString stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];

		//as it's impossible to show settings window directrly from our app, call one throught openURL
		[[UIApplication sharedApplication] openURL:[NSURL URLWithString:encodedString]];
		
		NSLog(@"Device is unable to send email in its current state.");
		return false;
	}
}
@end