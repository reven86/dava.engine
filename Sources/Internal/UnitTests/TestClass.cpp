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

#include "TestClass.h"

#if defined(__DAVAENGINE_APPLE__) || defined(__DAVAENGINE_ANDROID__)
#   include <cxxabi.h>
#endif

namespace DAVA
{
namespace UnitTests
{

String TestClass::PrettifyTypeName(const String& name) const
{
    String result = name;
#if defined(__DAVAENGINE_APPLE__) || defined(__DAVAENGINE_ANDROID__)
    // abi::__cxa_demangle reference
    // https://gcc.gnu.org/onlinedocs/libstdc++/libstdc++-html-USERS-4.3/a01696.html
    int status = 0;
    char* demangledName = abi::__cxa_demangle(name.c_str(), nullptr, nullptr, &status);
    if (demangledName != nullptr)
    {
        result = demangledName;
        free(demangledName);
    }
#endif

    size_t spacePos = result.find_last_of(": ");
    if (spacePos != String::npos)
    {
        return result.substr(spacePos + 1);
    }
    return result;
}

String TestClass::RemoveTestPostfix(const String& name) const
{
    String lowcase = name;
    // Convert name to lower case
    std::transform(lowcase.begin(), lowcase.end(), lowcase.begin(), [](char ch) -> char { return 'A' <= ch && ch <= 'Z' ? ch - 'A' + 'a' : ch; });
    size_t pos = lowcase.rfind("test");
    // If name ends with 'test' discard it
    if (pos != String::npos && pos > 0 && lowcase.length() - pos == 4)
    {
        return name.substr(0, pos);
    }
    return name;
}

}   // namespace UnitTests
}   // namespace DAVA
