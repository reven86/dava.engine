#pragma once

#include "Base/BaseTypes.h"
#include "Clipboard/Private/IClipboardImpl.h"

namespace DAVA
{
class ClipboardImplOsx : public IClipboardImpl
{
public:
    ClipboardImplOsx();
    ~ClipboardImplOsx() override;
    bool IsReadyToUse() const override;
    bool Clear() const override;
    bool HasText() const override;
    bool SetText(const WideString& str) override;
    WideString GetText() const override;
};

using ClipboardImpl = ClipboardImplOsx;
}
