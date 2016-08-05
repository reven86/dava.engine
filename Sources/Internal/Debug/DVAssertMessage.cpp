#include "Debug/DVAssertMessage.h"
#include "Core/Core.h"

#include "Concurrency/Atomic.h"
#include "Engine/Public/Engine.h"

namespace DAVA
{
namespace DVAssertMessage
{
namespace
{
Atomic<uint32> messageDisplayed{ 0 };
Function<bool(DVAssertMessage::eModalType, const char8*)> innerShowOverride;
}

#if defined(ENABLE_ASSERT_MESSAGE)

bool ShowMessage(eModalType modalType, const char8* text, ...)
{
    bool userClickBreak = false;
    // we don't need to show assert window for console mode
#if defined(__DAVAENGINE_COREV2__)
    if (Engine::Instance()->IsConsoleMode())
#else
    if (Core::Instance()->IsConsoleMode())
#endif
        return userClickBreak; // TODO what to do here? is loging only in console mode?

    va_list vl;
    va_start(vl, text);

    char tmp[4096] = { 0 };
    // sizeof(tmp) - 2  - We need two characters for appending "\n" if the number of characters exceeds the size of buffer.
    vsnprintf(tmp, sizeof(tmp) - 2, text, vl);
    strcat(tmp, "\n");
    messageDisplayed.Increment();
    if (innerShowOverride)
        userClickBreak = innerShowOverride(modalType, tmp);
    else
        userClickBreak = InnerShow(modalType, tmp);
    messageDisplayed.Decrement();
    va_end(vl);

    return userClickBreak;
}

#else

bool ShowMessage(eModalType /*modalType*/, const char8* /*text*/, ...)
{
    // Do nothing here.
    return false;
}

#endif // ENABLE_ASSERT_MESSAGE

bool IsHidden()
{
    return messageDisplayed.Get() == 0;
}

void SetShowInnerOverride(const DAVA::Function<bool(eModalType, const char8*)>& fn)
{
    innerShowOverride = fn;
}

} // namespace DVAssertMessage
} // namespace DAVA