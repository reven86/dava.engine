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


#ifndef REGKEY_H
#define REGKEY_H

#include "Base/BaseTypes.h"

class RegKey
{
public:
    RegKey(HKEY scope, const char* keyName, bool createIfNotExist = false);

    bool IsExist() const { return isExist; }
    bool IsCreated() const { return isCreated; }

    //TODO: replace on Optional<String>
    DAVA::String QueryString(const char* valueName) const;
    bool SetValue(const DAVA::String& valName, const DAVA::String& val);

    //TODO: replace on Optional<DWORD>
    DWORD QueryDWORD(const char* valueName) const;
    bool SetValue(const DAVA::String& valName, DWORD val);

    template <typename T>
    //TODO: replace on Optional<T>
    T QueryValue(const char* valueName);

private:
    bool isExist = false;
    bool isCreated = false;
    HKEY key;
};

template <>
inline DAVA::String RegKey::QueryValue<DAVA::String>(const char* valueName)
{
    return QueryString(valueName);
}

template <>
inline DWORD RegKey::QueryValue<DWORD>(const char* valueName)
{
    return QueryDWORD(valueName);
}

#endif  // REGKEY_H