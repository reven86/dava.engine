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


#include "RegKey.h"

namespace DAVA
{

RegKey::RegKey(HKEY scope, const char* keyName, bool createIfNotExist)
{
    long res = ::RegOpenKeyEx(scope, keyName, 0, KEY_READ | KEY_WOW64_64KEY, &key);

    if (res != ERROR_SUCCESS && createIfNotExist)
    {
        res = ::RegCreateKeyEx(
            scope, keyName, 0, 0, 0, KEY_WRITE | KEY_WOW64_64KEY, 0, &key, 0);
        isCreated = res == ERROR_SUCCESS;
    }

    isExist = res == ERROR_SUCCESS;
}

String RegKey::QueryString(const char* valueName) const
{
    Array<char, 1024> arr{};
    DWORD size = arr.size();
    DWORD type;

    ::RegQueryValueEx(key,
                      valueName,
                      NULL,
                      &type,
                      reinterpret_cast<LPBYTE>(arr.data()),
                      &size);

    return type == REG_SZ ? arr.data() : "";
}

bool RegKey::SetValue(const String& valName, const String& val)
{
    long res = ::RegSetValueEx(key, valName.c_str(), 0, REG_SZ,
        (LPBYTE)val.c_str(), val.size() + 1);
    return res == ERROR_SUCCESS;
}

DWORD RegKey::QueryDWORD(const char* valueName) const
{
    DWORD result;
    DWORD size = sizeof(result);
    DWORD type;

    ::RegQueryValueEx(key,
                      valueName,
                      NULL,
                      &type,
                      reinterpret_cast<LPBYTE>(&result),
                      &size);

    return type == REG_DWORD ? result : -1;
}

bool RegKey::SetValue(const String& valName, DWORD val)
{
    long res = ::RegSetValueEx(key, valName.c_str(), 0, REG_DWORD,
        (LPBYTE)&val, sizeof(DWORD));
    return res == ERROR_SUCCESS;
}

}  // namespace DAVA