/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "ClipboardImplWin32.h"

#include <WinUser.h>

namespace DAVA
{
ClipboardImplWin32::ClipboardImplWin32()
{
    isReady = ::OpenClipboard(nullptr) != 0;
}

ClipboardImplWin32::~ClipboardImplWin32()
{
    if (isReady)
    {
        ::CloseClipboard();
    }
}

bool ClipboardImplWin32::IsReadyToUse() const
{
    return isReady;
}

void ClipboardImplWin32::ClearClipboard() const
{
    if (isReady)
    {
        ::EmptyClipboard();
    }
}

void ClipboardImplWin32::SetWideString(const WideString& str) const
{
    if (isReady)
    {
        ClearClipboard();
        auto length = str.length();
        auto hglbCopy = ::GlobalAlloc(GMEM_MOVEABLE, (length + 1) * sizeof(WideString::value_type));
        auto lptstrCopy = static_cast<LPWSTR>(::GlobalLock(hglbCopy));
        Memcpy(lptstrCopy, str.c_str(), length * sizeof(WideString::value_type));
        lptstrCopy[length] = static_cast<WideString::value_type>(0); // null character
        ::GlobalUnlock(hglbCopy);
        ::SetClipboardData(CF_UNICODETEXT, hglbCopy);
    }
}

WideString ClipboardImplWin32::GetWideString() const
{
    WideString outPut;
    if (isReady && ::IsClipboardFormatAvailable(CF_UNICODETEXT))
    {
        auto hglb = ::GetClipboardData(CF_UNICODETEXT);
        if (hglb != nullptr)
        {
            auto lptstr = static_cast<LPWSTR>(::GlobalLock(hglb));
            if (lptstr != NULL)
            {
                outPut = WideString(lptstr);
                ::GlobalUnlock(hglb);
            }
        }
    }
    return outPut;
}

void ClipboardImplWin32::SetString(const String& str) const
{
    if (isReady)
    {
        ClearClipboard();
        auto length = str.length();
        auto hglbCopy = ::GlobalAlloc(GMEM_MOVEABLE, (length + 1) * sizeof(WideString::value_type));
        auto lptstrCopy = static_cast<LPSTR>(::GlobalLock(hglbCopy));
        Memcpy(lptstrCopy, str.c_str(), length * sizeof(WideString::value_type));
        lptstrCopy[length] = static_cast<WideString::value_type>(0); // null character
        ::GlobalUnlock(hglbCopy);
        ::SetClipboardData(CF_TEXT, hglbCopy);
    }
}

String ClipboardImplWin32::GetString() const
{
    String outPut;
    if (isReady && ::IsClipboardFormatAvailable(CF_TEXT))
    {
        auto hglb = ::GetClipboardData(CF_TEXT);
        if (hglb != nullptr)
        {
            auto lptstr = static_cast<LPSTR>(::GlobalLock(hglb));
            if (lptstr != NULL)
            {
                outPut = String(lptstr);
                ::GlobalUnlock(hglb);
            }
        }
    }
    return outPut;
}
}
