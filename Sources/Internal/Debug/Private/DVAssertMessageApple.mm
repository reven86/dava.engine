#include "Debug/DVAssertMessage.h"

#if !defined(__DAVAENGINE_COREV2__)

#if defined(__DAVAENGINE_MACOS__) 
#import <Foundation/Foundation.h>
#include <AppKit/NSAlert.h>
#elif defined(__DAVAENGINE_IPHONE__)
#include "UI/UIScreenManager.h"
#import "UIAlertView_Modal.h"
#import "UIDismissionHandlerAlertView.h"

UIDismissionHandlerAlertView* messageBoxPtr = nil;
#endif

bool DAVA::DVAssertMessage::InnerShow(eModalType modalType, const char* content)
{
    bool breakExecution = false;
#if defined(__DAVAENGINE_MACOS__)
    NSString* contents = [NSString stringWithUTF8String:content];

    NSAlert* alert = [[NSAlert alloc] init];
    [alert setMessageText:@"Assert"];
    [alert setInformativeText:contents];
    [alert addButtonWithTitle:@"Ok"];
    if (ALWAYS_MODAL == modalType)
    {
        [alert addButtonWithTitle:@"Break"];
    }
    // Modal Types are ignored on MacOS - there is no way to show non-modal alerts on this platform.
    if ([alert runModal] == NSAlertSecondButtonReturn)
    {
        breakExecution = true;
    }
    [alert release];
//VI: no need to release contents since its created on autorelease pool
//[contents release];
#elif defined(__DAVAENGINE_IPHONE__)
    NSString* contents = [NSString stringWithUTF8String:content];

    switch (modalType)
    {
    case ALWAYS_MODAL:
    {
        // Yuri Coder, 2013/02/06. This method is specific for iOS-implementation only,
        // it blocks drawing to avoid deadlocks. See RenderView.mm file for details.
        UIScreenManager::Instance()->BlockDrawing();

        // Yuri Coder, 2013/07/19. Always display new Alert View in case of ASSERT.
        UIAlertView* alert = [[UIAlertView alloc] initWithTitle:@"Assert" message:contents delegate:nil cancelButtonTitle:@"Ok" otherButtonTitles:@"Break", nil];

        long breakButtonIndex = [alert firstOtherButtonIndex];

        [alert performSelectorOnMainThread:@selector(showModal) withObject:nil waitUntilDone:YES];

        if ([alert getClickedButtonIndex] == breakButtonIndex)
        {
            breakExecution = true;
        }

        UIScreenManager::Instance()->UnblockDrawing();
        break;
    }

    default:
    {
        // if we already open one Alert messagebox skip all new
        // while waiting user click Ok
        if (nil == messageBoxPtr)
        {
            // Create the new alert message and show it.
            UIDismissionHandlerAlertView* alert =
            [[[UIDismissionHandlerAlertView alloc] initWithTitle:@"Assert"
                                                         message:contents
                                               cancelButtonTitle:@"OK"
                                               otherButtonTitles:nil] autorelease];
            messageBoxPtr = alert;
            [alert showWithDismissHandler:^(NSInteger selectedIndex, BOOL didCancel) {
              messageBoxPtr = nil;
            }];
        }
    }
    }
    
#endif
    return breakExecution;
}
#endif // !defined(__DAVAENGINE_COREV2__)
