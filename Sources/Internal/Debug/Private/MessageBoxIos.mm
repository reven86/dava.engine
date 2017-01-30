#include "Base/Platform.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#include "Base/BaseTypes.h"
#include "Concurrency/Semaphore.h"
#include "Concurrency/AutoResetEvent.h"
#include "Debug/DVAssert.h"

#import <Foundation/NSThread.h>
#import <UIKit/UIAlertView.h>

bool showingMessageBox = false;

@interface AlertDialog : NSObject<UIAlertViewDelegate>

- (int)showModal;
- (void)addButtonWithTitle:(NSString*)buttonTitle;
- (void)alertView:(UIAlertView*)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex;

@property(nonatomic, assign) NSString* title;
@property(nonatomic, assign) NSString* message;
@property(nonatomic, assign) NSMutableArray<NSString*>* buttonNames;
@property(nonatomic, readonly) int clickedIndex;

@end

@implementation AlertDialog

- (int)showModal
{
    showingMessageBox = true;

    @autoreleasepool
    {
        UIAlertView* alert = [[[UIAlertView alloc] initWithTitle:_title
                                                         message:_message
                                                        delegate:self
                                               cancelButtonTitle:nil
                                               otherButtonTitles:nil, nil] autorelease];
        for (NSString* s : _buttonNames)
        {
            [alert addButtonWithTitle:s];
        }

        _clickedIndex = -1;
        [alert show];
        @autoreleasepool
        {
            while (_clickedIndex < 0)
            {
                [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
            }
        }
        [alert setDelegate:nil];
    }

    showingMessageBox = false;
    return _clickedIndex;
}

- (void)addButtonWithTitle:(NSString*)buttonTitle
{
    if (_buttonNames == nil)
    {
        _buttonNames = [[NSMutableArray alloc] init];
    }
    [_buttonNames addObject:buttonTitle];
}

- (void)alertView:(UIAlertView*)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
{
    _clickedIndex = static_cast<int>(buttonIndex);
}

@end

namespace DAVA
{
namespace Debug
{
Semaphore semaphore(1);
AutoResetEvent autoEvent;

int MessageBox(const String& title, const String& message, const Vector<String>& buttons, int defaultButton)
{
    DVASSERT(0 < buttons.size() && buttons.size() <= 3);
    DVASSERT(0 <= defaultButton && defaultButton < static_cast<int>(buttons.size()));

    int result = -1;
    auto showMessageBox = [title, message, buttons, defaultButton, &result]() {
        @autoreleasepool
        {
            AlertDialog* alertDialog = [[[AlertDialog alloc] init] autorelease];
            [alertDialog setTitle:@(title.c_str())];
            [alertDialog setMessage:@(message.c_str())];

            for (const String& s : buttons)
            {
                [alertDialog addButtonWithTitle:@(s.c_str())];
            }

            [alertDialog performSelectorOnMainThread:@selector(showModal)
                                          withObject:nil
                                       waitUntilDone:YES];
            result = [alertDialog clickedIndex];
            autoEvent.Signal();
        }
    };

    const bool directCall = [NSThread isMainThread];
    if (directCall)
    {
        showMessageBox();
    }
    else
    {
        // Do not use Window::RunOnUIThread as message box becomes unresponsive to user input.
        // I do not know why so.
        semaphore.Wait();
        showMessageBox();
        autoEvent.Wait();
        semaphore.Post(1);
    }
    return result;
}

} // namespace Debug
} // namespace DAVA

#endif // defined(__DAVAENGINE_IPHONE__)
#endif // defined(__DAVAENGINE_COREV2__)
