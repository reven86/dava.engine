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

#include "Utils.h"

#if defined (__DAVAENGINE_WINDOWS__)
#   include <Objbase.h>
#   include <AtlConv.h>

#   if defined (__DAVAENGINE_WIN32__)
#       include <ShellAPI.h>
#   elif defined (__DAVAENGINE_WIN_UAP__)
#       include <ppltasks.h>
#   endif

namespace DAVA 
{

String GenerateGUID()
{
    //create new GUID
    GUID guid;
    if (FAILED(CoCreateGuid(&guid)))
        return "";

    //get string representation of GUID
    Array<OLECHAR, 64> guidString {};
    ::StringFromGUID2(guid, guidString.data(), static_cast<int>(guidString.size()));
    
    //convert to normal string
    USES_CONVERSION;
    return OLE2CA(guidString.data());
}

#   if defined (__DAVAENGINE_WIN_UAP__)

void OpenURL(const String& url)
{
	auto platform_string = ref new Platform::String(StringToWString(url).c_str());
	auto uri = ref new Windows::Foundation::Uri(platform_string);
	concurrency::task<bool> launchUriOperation(Windows::System::Launcher::LaunchUriAsync(uri));
	launchUriOperation.get();
}

#   elif defined (__DAVAENGINE_WIN32__)

void OpenURL(const String& url)
{
	WideString urlWide = StringToWString(url);
	ShellExecute(NULL, L"open", urlWide.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

#   endif //  __DAVAENGINE_WIN32__

} //  namespace DAVA

#endif //  __DAVAENGINE_WINDOWS__