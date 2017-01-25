#include "Base/Platform.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_QT__)
#elif defined(__DAVAENGINE_MACOS__)

#include "Base/BaseTypes.h"
#include "Engine/Engine.h"
#include "Debug/DVAssert.h"
#include "Utils/NSStringUtils.h"

#import <AppKit/NSAlert.h>
#import <Foundation/NSThread.h>

namespace DAVA
{
namespace Debug
{
int MessageBox(const String& title, const String& message, const Vector<String>& buttons, int defaultButton)
{
    DVASSERT(0 < buttons.size() && buttons.size() <= 3);
    DVASSERT(0 <= defaultButton && defaultButton < static_cast<int>(buttons.size()));

    int result = -1;
    auto showMessageBox = [&title, &message, &buttons, defaultButton, &result]() {
        @autoreleasepool
        {
            NSAlert* alert = [[NSAlert alloc] init];
            [alert setMessageText:NSStringFromString(title)];
            [alert setInformativeText:NSStringFromString(message)];

            int i = 0;
            for (const String& s : buttons)
            {
                NSButton* alertButton = [alert addButtonWithTitle:NSStringFromString(s)];
                if (i == defaultButton)
                    [alertButton setKeyEquivalent:@"\r"];
                else
                    [alertButton setKeyEquivalent:@""];
                i += 1;
            }

            NSModalResponse response = [alert runModal];
            switch (response)
            {
            case NSAlertFirstButtonReturn:
                result = 0;
                break;
            case NSAlertSecondButtonReturn:
                result = 1;
                break;
            case NSAlertThirdButtonReturn:
                result = 2;
                break;
            default:
                result = 0;
                break;
            }
            [alert release];
        }
    };

    Window* primaryWindow = GetPrimaryWindow();
    const bool directCall = primaryWindow == nullptr || [NSThread isMainThread];
    if (directCall)
    {
        showMessageBox();
    }
    else
    {
        primaryWindow->RunOnUIThread(showMessageBox);
    }
    return result;
}

} // namespace Debug
} // namespace DAVA

#endif // defined(__DAVAENGINE_MACOS__)
#endif // defined(__DAVAENGINE_COREV2__)
