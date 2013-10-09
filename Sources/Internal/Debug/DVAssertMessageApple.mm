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

#include "Debug/DVAssertMessage.h"

#if defined(__DAVAENGINE_MACOS__) 
#import <Foundation/Foundation.h>
#include <AppKit/NSAlert.h>
#elif defined(__DAVAENGINE_IPHONE__)
#include "UI/UIScreenManager.h"
#import "UIAlertView_Modal.h"
#import "UIDismissionHandlerAlertView.h"
#endif


void DAVA::DVAssertMessage::InnerShow(eModalType modalType, const char* content)
{
#if defined(__DAVAENGINE_MACOS__)
    NSString *contents = [NSString stringWithUTF8String:content];
    
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:@"Assert"];
    [alert setInformativeText:contents];
	
	// Modal Types are ignored on MacOS - there is no way to show non-modal alerts on this platform.
    [alert runModal];
    [alert release];
	//VI: no need to release contents since its created on autorelease pool
    //[contents release];
#elif defined(__DAVAENGINE_IPHONE__)
	static const uint MAX_WARNING_MESSAGE_LENGTH = 8192;
    NSString *contents = [NSString stringWithUTF8String:content];

	switch (modalType)
	{
		case ALWAYS_MODAL:
		{
			// Yuri Coder, 2013/02/06. This method is specific for iOS-implementation only,
			// it blocks drawing to avoid deadlocks. See EAGLView.mm file for details.
			UIScreenManager::Instance()->BlockDrawing();
			
			// Yuri Coder, 2013/07/19. Always display new Alert View in case of ASSERT.
			UIAlertView* alert = [[UIAlertView alloc] initWithTitle:@"Assert" message:contents delegate:nil cancelButtonTitle:@"OK" otherButtonTitles: nil];
			[alert performSelectorOnMainThread:@selector(showModal) withObject:nil waitUntilDone:YES];
			break;
		}

		default:
		{
			bool needRecreateAlertView = true;
			if (messageBoxPtr)
			{
				// An alert already opened - dismiss it and re-create with the new message.
				UIDismissionHandlerAlertView* alert = (UIDismissionHandlerAlertView*)messageBoxPtr;
				NSString* curAlertMessage = [alert getMessage];
				if (curAlertMessage)
				{
					contents = [curAlertMessage stringByAppendingFormat:@"%@\n", contents];
				}
				
				// Don't allow too long strings.
				if ([contents length] <= MAX_WARNING_MESSAGE_LENGTH)
				{
					[alert dismiss:NO];
					messageBoxPtr = NULL;
				}
				else
				{
					// Leave the current Alert View as-is.
					needRecreateAlertView = false;
					return;
				}
			}

			if (needRecreateAlertView)
			{
				// Create the new alert message and show it.
				UIDismissionHandlerAlertView* alert =
					[[[UIDismissionHandlerAlertView alloc] initWithTitle:@"Assert" message:contents
												   cancelButtonTitle:@"OK" otherButtonTitles: nil] autorelease];
				messageBoxPtr = alert;
				[alert showWithDismissHandler:^(NSInteger selectedIndex, BOOL didCancel)
				 {
					 messageBoxPtr = NULL;
				 }];
			}
		}
	}
    
#endif

}

