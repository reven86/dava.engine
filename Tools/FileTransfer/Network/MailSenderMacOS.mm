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

#include "Network/MailSender.h"

#if defined(__DAVAENGINE_IPHONE__)
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "MailComposeDelegate.h"
#import "UIAlertView_Modal.h"
#include "UIScreenManageriPhone.h"
#include "EAGLViewController.h"

#elif defined(__DAVAENGINE_MACOS__)
#import <Foundation/Foundation.h>
#import <AppKit/NSWorkspace.h>
#endif

namespace DAVA
{
	
#if defined(__DAVAENGINE_IPHONE__)
	
class MailSenderApplePlatform
{
public:
	static bool SendEmail(NSString* msgEmail, NSString* msgSubj, NSString* msgBody)
	{
		EAGLViewController* rootViewController = (EAGLViewController *)UIScreenManager::Instance()->GetController();
		
		MailComposeDelegate* mailDelegate = [[[MailComposeDelegate alloc] initWithController:rootViewController
																					   email:msgEmail
																					 subject:msgSubj
																					 message:msgBody] autorelease];
		
		return [mailDelegate show];
	}
};
	
#elif defined(__DAVAENGINE_MACOS__)
	
class MailSenderApplePlatform
{
public:
	static bool SendEmail(NSString* msgEmail, NSString* msgSubj, NSString* msgBody)
	{
		// Build mailto string
		NSString* mailtoString = [NSString stringWithFormat:@"mailto:?to=%@&subject=%@&body=%@",
								  msgEmail, msgSubj, msgBody];
		// Build correct web string without special charachters
		NSString* encodedString = [mailtoString stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
		return [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:encodedString]];
	}
};
	
#endif

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
	bool MailSender::SendEmail(const WideString &email, const WideString &subject, const WideString &messageText)
	{
		// Don't try to open mail client if empty email string is passed
		if (email.empty())
			return false;
		
		// Convert input values into NSString
		NSString* msgEmail = [[[NSString alloc] initWithBytes: email.data()
													   length: email.size() * sizeof(wchar_t)
													 encoding:NSUTF32LittleEndianStringEncoding] autorelease];
		NSString* msgSubj = [[[NSString alloc] initWithBytes: subject.data()
													  length: subject.size() * sizeof(wchar_t)
													encoding:NSUTF32LittleEndianStringEncoding] autorelease];
		NSString* msgBody = [[[NSString alloc] initWithBytes: messageText.data()
													  length: messageText.size() * sizeof(wchar_t)
													encoding:NSUTF32LittleEndianStringEncoding] autorelease];

		
		return MailSenderApplePlatform::SendEmail(msgEmail, msgSubj, msgBody);
	}
	
#endif
}

