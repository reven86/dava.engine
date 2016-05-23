#ifndef __DAVAENGINE_CLIPBOARDIMPLWIN32_H__
#define __DAVAENGINE_CLIPBOARDIMPLWIN32_H__

#include "Base/BaseTypes.h"
#include "Clipboard/Private/IClipboardImpl.h"

namespace DAVA
{
class ClipboardImplWin32 : public IClipboardImpl
{
public:
    ClipboardImplWin32();
    ~ClipboardImplWin32() override;
    bool IsReadyToUse() const override;
    bool ClearClipboard() const override;
    bool HasText() const override;
    bool SetText(const WideString& str) override;
    WideString GetText() const override;

private:
    bool isReady = false;
};

using ClipboardImpl = ClipboardImplWin32;
}

#endif //__DAVAENGINE_CLIPBOARDIMPLWIN32_H__
