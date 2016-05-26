#include "Clipboard/Clipboard.h"

#if defined(__DAVAENGINE_WIN32__)
#include "win32/ClipboardImplWin32.h"
#else
#include "ClipboardImplStub.h"
#endif

namespace DAVA
{
Clipboard::Clipboard()
{
    pImpl = new ClipboardImpl();
}

Clipboard::~Clipboard()
{
    SafeDelete(pImpl);
}

bool Clipboard::IsReadyToUse() const
{
    return pImpl->IsReadyToUse();
}

bool Clipboard::ClearClipboard() const
{
    return pImpl->ClearClipboard();
}

bool Clipboard::HasText() const
{
    return pImpl->HasText();
}

bool Clipboard::SetText(const WideString& str)
{
    return pImpl->SetText(str);
}

WideString Clipboard::GetText() const
{
    return pImpl->GetText();
}
}
