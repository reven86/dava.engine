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


#include "Platform/DateTime.h"
#include "Utils/UTF8Utils.h"
#include "Utils/Utils.h"
#include "FileSystem/LocalizationSystem.h"

namespace DAVA
{
	DAVA::WideString DateTime::AsWString(const wchar_t* format) const
	{
        String configLocale = LocalizationSystem::Instance()->GetCountryCode();
        WideString configLocaleWide = StringToWString(configLocale);

		configLocale.replace(configLocale.find("_"), 1, "-");
        int nchars = GetLocaleInfoEx(configLocaleWide.c_str(), LOCALE_SENGLANGUAGE, nullptr, 0);
        wchar_t languageCode[8] {};
		GetLocaleInfoEx(configLocaleWide.c_str(), LOCALE_SENGLANGUAGE, languageCode, nchars);

		struct tm timeinfo {};
        wchar_t buffer [256] {};
		
        Timestamp timeWithTZ = innerTime + timeZoneOffset;

		GmTimeThreadSafe(&timeinfo, &timeWithTZ);

        _locale_t loc = _create_locale(LC_ALL, UTF8Utils::EncodeToUTF8(languageCode).c_str());
		DVASSERT(loc);
        _wcsftime_l(buffer, 256, format, &timeinfo, loc);

        DAVA::WideString str(buffer);
        _free_locale(loc);
        return str;
    }

    int32 DateTime::GetLocalTimeZoneOffset()
    {
		TIME_ZONE_INFORMATION TimeZoneInfo;
		GetTimeZoneInformation( &TimeZoneInfo );
	
		// TimeZoneInfo.Bias is the difference between local time
		// and GMT in minutes.
        return TimeZoneInfo.Bias*(-60);
    }
}