#ifndef __DAVAENGINE_CLIPBOARDIMPLSTUB_H__
#define __DAVAENGINE_CLIPBOARDIMPLSTUB_H__

#include "Base/BaseTypes.h"
#include "Clipboard/Private/IClipboardImpl.h"

namespace DAVA
{
class ClipboardImplStub : public IClipboardImpl
{
public:
    ClipboardImplStub() = default;
    ~ClipboardImplStub() override = default;
    bool IsReadyToUse() const override;
    bool ClearClipboard() const override;
    bool HasText() const override;
    bool SetText(const WideString& str) override;
    WideString GetText() const override;
};

using ClipboardImpl = ClipboardImplStub;

inline bool ClipboardImplStub::IsReadyToUse() const
{
    return false;
}

inline bool ClipboardImplStub::ClearClipboard() const
{
    return false;
}

inline bool ClipboardImplStub::HasText() const
{
    return false;
}

inline bool ClipboardImplStub::SetText(const WideString& str)
{
    return false;
}

inline WideString ClipboardImplStub::GetText() const
{
    return WideString();
}
}

#endif //__DAVAENGINE_CLIPBOARDIMPLSTUB_H__
