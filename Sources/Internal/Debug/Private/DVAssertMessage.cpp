#include "Debug/DVAssertMessage.h"

#include "Core/Core.h"

#include "Concurrency/Atomic.h"
#include "Engine/Engine.h"

#include "Debug/MessageBox.h"

namespace DAVA
{
namespace DVAssertMessage
{
#if !defined(__DAVAENGINE_COREV2__)
namespace
{
Atomic<uint32> messageDisplayed{ 0 };
Function<bool(DVAssertMessage::eModalType, const char8*)> innerShowOverride;
}
#endif

DAVA_DEPRECATED(bool ShowMessage(eModalType modalType, const char8* text, ...))
{
    bool userClickBreak = false;

#if !defined(__DAVAENGINE_COREV2__)
    // we don't need to show assert window for console mode
    if (Core::Instance()->IsConsoleMode())
        return userClickBreak; // TODO what to do here? is loging only in console mode?
#endif

    va_list vl;
    va_start(vl, text);

    char tmp[4096] = { 0 };
    // sizeof(tmp) - 2  - We need two characters for appending "\n" if the number of characters exceeds the size of buffer.
    vsnprintf(tmp, sizeof(tmp) - 2, text, vl);
    va_end(vl);
    strcat(tmp, "\n");

#if defined(__DAVAENGINE_COREV2__)
    int choice = Debug::MessageBox("Assert", tmp, { "Break", "Continue" }, 0);
    userClickBreak = choice <= 0; // If by some reason MessageBox cannot be shown or is shown non-modal break execution
#else
    messageDisplayed.Increment();
    if (innerShowOverride)
        userClickBreak = innerShowOverride(modalType, tmp);
    else
        userClickBreak = InnerShow(modalType, tmp);
    messageDisplayed.Decrement();
#endif

    return userClickBreak;
}

#if !defined(__DAVAENGINE_COREV2__)
bool IsHidden()
{
    return messageDisplayed.Get() == 0;
}

void SetShowInnerOverride(const DAVA::Function<bool(eModalType, const char8*)>& fn)
{
    innerShowOverride = fn;
}
#endif

} // namespace DVAssertMessage
} // namespace DAVA
