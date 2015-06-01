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


#include "DAVAEngine.h"

#include "Infrastructure/GameCore.h"
#include "Infrastructure/NewTestFramework.h"

using namespace DAVA;

DAVA_TESTCLASS(DateTimeTest)
{
    DAVA_TEST(TestFunction)
    {
        DAVA::DateTime date = DAVA::DateTime::Now();
        Logger::Debug("#Getting of current date:");
        PrintDateTimeContent(date);

        date = DAVA::DateTime(2001, 1, 1, 14, 0, 0, 5 * 3600);
        Logger::Debug("#Creating of date of the 1st of Feb 2001, 14:00:00, utc+5:");
        PrintDateTimeContent(date);

        date = date.ConvertToLocalTimeZone();
        Logger::Debug("#Changing to local tz:");
        PrintDateTimeContent(date);

        date = date.ConvertToTimeZone(-3 * 3600);;
        Logger::Debug("#Changing to utc-3:");
        PrintDateTimeContent(date);

        DAVA::DateTime gmDate = DateTime::GmTime(date.GetTimestamp());
        Logger::Debug("#GmTime() for the timestamp of <1st of Feb 2001, 9:00:00> call:");
        PrintDateTimeContent(gmDate);

        DAVA::DateTime localDate = DateTime::LocalTime(date.GetTimestamp());
        Logger::Debug("#LocalTime() for the timestamp of <1st of Feb 2001, 9:00:00> call:");
        PrintDateTimeContent(localDate);

        date.ParseISO8601Date("1970-01-01T05:00:00-03:00");
        Logger::Debug("#Parcing of <1970-01-01T05:00:00-03:00>:");
        PrintDateTimeContent(date);

        date.ParseRFC822Date("Wed, 27 Sep 2006 21:36:45 +0100");
        Logger::Debug("#Parcing of <Wed, 27 Sep 2006 21:36:45 +0100>:");
        PrintDateTimeContent(date);

        Logger::Debug("********** DateTime test **********");

        TEST_VERIFY(true);
    }

    void PrintDateTimeContent(const DateTime& inputTime)
    {
        int32 y = inputTime.GetYear();
        int32 month = inputTime.GetMonth();
        int32 day = inputTime.GetDay();
        int32 hour = inputTime.GetHour();
        int32 minute = inputTime.GetMinute();
        int32 sec = inputTime.GetSecond();
        int32 tz = inputTime.GetTimeZoneOffset() / 60;
        Logger::Debug("\tContent of current date by components:");
        Logger::Debug("\tYear:%d Month:%d DayOfMonth(counting from 1):%d Hour:%d Minute:%d Second:%d timZoneOffset(in minutes): %d", y, month, day, hour, minute, sec, tz);
    }
};
