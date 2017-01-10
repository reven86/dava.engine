#ifndef __DAVAENGINE_HELPER_APP_DELEGATE_MAC_H__
#define __DAVAENGINE_HELPER_APP_DELEGATE_MAC_H__


#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_MACOS__)

#if !defined(__DAVAENGINE_COREV2__)

#include "Core/ApplicationCore.h"
#import "Platform/TemplateMacOS/MainWindowController.h"

#import <AppKit/AppKit.h>

@interface HelperAppDelegate : NSObject<NSApplicationDelegate>
{
@private
    MainWindowController* mainWindowController;
}

- (void)setWindowController:(MainWindowController*)ctrlr;

@end

#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_HELPER_APP_DELEGATE_MAC_H__
