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
#include "FileSystem/LocalizationSystem.h"

#include <time.h>
#include <xlocale.h>

#import <Foundation/Foundation.h>

#include "Utils/UTF8Utils.h"

namespace DAVA
{
    DAVA::WideString DateTime::AsWString(const wchar_t* format) const
    {
        DAVA::String locID = LocalizationSystem::Instance()->GetCountryCode();
        
        struct tm timeinfo;
        wchar_t buffer [256] = {0};
        
        Timestamp timeWithTZ = innerTime + timeZoneOffset;
        
        GmTimeThreadSafe(&timeinfo, &timeWithTZ);
        
        locale_t loc = newlocale(LC_ALL_MASK, locID.c_str(), NULL);
        size_t size = wcsftime_l(buffer, 256, format, &timeinfo, loc);
        DVASSERT(size);
        DAVA::WideString str(buffer);
        
        return str;
    }

    WideString DateTime::GetLocalizedDate() const
    {
        time_t timeWithTZ = innerTime + timeZoneOffset;

        tm timeinfo;
        GmTimeThreadSafe(&timeinfo, &timeWithTZ);

        DAVA::String locID = LocalizationSystem::Instance()->GetCountryCode();

        NSString* locale_name = [NSString stringWithCString:locID.c_str()
                                                   encoding:[NSString defaultCStringEncoding]];

        NSLocale* locale = [[NSLocale alloc] initWithLocaleIdentifier:locale_name];

        NSDate* date = [NSDate dateWithTimeIntervalSince1970:timeWithTZ];

        NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
        dateFormatter.timeStyle = NSDateFormatterNoStyle;
        dateFormatter.dateStyle = NSDateFormatterShortStyle;

        [dateFormatter setLocale:locale];

        NSString* date_str = [dateFormatter stringFromDate:date];

        std::string result = [date_str cStringUsingEncoding:[NSString defaultCStringEncoding]];

        return UTF8Utils::EncodeToWideString(result);
    }

    WideString DateTime::GetLocalizedTime() const
    {
        time_t timeWithTZ = innerTime + timeZoneOffset;

        tm timeinfo;
        GmTimeThreadSafe(&timeinfo, &timeWithTZ);

        DAVA::String locID = LocalizationSystem::Instance()->GetCountryCode();

        NSString* locale_name = [NSString stringWithCString:locID.c_str()
                                                   encoding:[NSString defaultCStringEncoding]];

        NSLocale* locale = [[NSLocale alloc] initWithLocaleIdentifier:locale_name];

        NSDate* date = [NSDate dateWithTimeIntervalSince1970:timeWithTZ];

        NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
        dateFormatter.timeStyle = NSDateFormatterMediumStyle;
        dateFormatter.dateStyle = NSDateFormatterNoStyle;

        [dateFormatter setLocale:locale];

        NSString* date_str = [dateFormatter stringFromDate:date];

        std::string result = [date_str cStringUsingEncoding:[NSString defaultCStringEncoding]];

        return UTF8Utils::EncodeToWideString(result);
    }

    int32 DateTime::GetLocalTimeZoneOffset()
    {
        Timestamp  t = time(NULL);
        struct tm resultStruct = {0};
        DateTime::LocalTimeThreadSafe(&resultStruct, &t);
        return (int32)resultStruct.tm_gmtoff;
    }
}
