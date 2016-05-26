#include "Debug/DVAssertMessage.h"
#include "Core/Core.h"

using namespace DAVA;

namespace
{
Atomic<uint32> messageDisplayed{ 0 };
}

#if defined(ENABLE_ASSERT_MESSAGE)

bool DVAssertMessage::ShowMessage(eModalType modalType, const char8* text, ...)
{
    bool userClickBreak = false;
    // we don't need to show assert window for console mode
    if (Core::Instance()->IsConsoleMode())
        return userClickBreak; // TODO what to do here? is loging only in console mode?

    va_list vl;
    va_start(vl, text);

    char tmp[4096] = { 0 };
    // sizeof(tmp) - 2  - We need two characters for appending "\n" if the number of characters exceeds the size of buffer.
    vsnprintf(tmp, sizeof(tmp) - 2, text, vl);
    strcat(tmp, "\n");
    messageDisplayed.Increment();
    userClickBreak = InnerShow(modalType, tmp);
    messageDisplayed.Decrement();
    va_end(vl);

    return userClickBreak;
}


#else

bool DVAssertMessage::ShowMessage(eModalType /*modalType*/, const char8* /*text*/, ...)
{
    // Do nothing here.
    return false;
}

#endif // ENABLE_ASSERT_MESSAGE

bool DVAssertMessage::IsHidden()
{
    return messageDisplayed.Get() == 0;
}
