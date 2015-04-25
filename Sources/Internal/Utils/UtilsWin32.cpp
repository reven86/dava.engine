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

namespace DAVA {

String GenerateGUID()
{
	Logger::Warning("GenerateGUID for Win32 Not implemented");
	//TO::DO
	return String();
}
};

#endif //  __DAVAENGINE_WINDOWS__

#if defined (__DAVAENGINE_WINDOWS_STORE__)
#include <ppltasks.h>

namespace DAVA {

void OpenURL(const String& url)
{
	auto platform_string = ref new Platform::String(StringToWString(url).c_str());
	auto uri = ref new Windows::Foundation::Uri(platform_string);
	concurrency::task<bool> launchUriOperation(Windows::System::Launcher::LaunchUriAsync(uri));
	launchUriOperation.get();
}
};

#elif defined (__DAVAENGINE_WINDOWS_DESKTOP__)

#include <Windows.h>
#include <ShellAPI.h>

namespace DAVA {

void OpenURL(const String& url)
{
	WideString urlWide = StringToWString(url);
	ShellExecute(NULL, L"open", urlWide.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
};

#endif //  __DAVAENGINE_WINDOWS_STORE__ | __DAVAENGINE_WINDOWS_DESKTOP__
