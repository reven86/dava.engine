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
#include "UnitTests/UnitTests.h"

using namespace DAVA;

DAVA_TESTCLASS(DateTimeTest)
{
    DAVA_TEST(TestFunction)
    {
        TEST_VERIFY(FormatDateTime(DateTime(1970, 0, 1, 0, 0, 0, 0)) == "1970-00-01 00:00:00+0");
        
        TEST_VERIFY(FormatDateTime(DateTime(2001, 1, 1, 0, 0, 0, 3 * 3600)) == "2001-01-01 00:00:00+10800");
        TEST_VERIFY(FormatDateTime(DateTime(2002, 2, 3, 5, 20, 10, -3 * 3600)) == "2002-02-03 05:20:10-10800");
        TEST_VERIFY(FormatDateTime(DateTime(2020, 11, 31, 5, 22, 10, -2 * 3600)) == "2020-11-31 05:22:10-7200");
        TEST_VERIFY(FormatDateTime(DateTime(2001, 1, 1, 14, 0, 0, 5 * 3600)) == "2001-01-01 14:00:00+18000");

        // Local timezone is UTC+3, so nothing should be changed
        TEST_VERIFY(FormatDateTime(DateTime(2001, 1, 1, 0, 0, 0, 3 * 3600).ConvertToLocalTimeZone()) == "2001-01-01 00:00:00+10800");
        TEST_VERIFY(FormatDateTime(DateTime(2001, 1, 1, 0, 0, 0, 3 * 3600).ConvertToTimeZone(0)) == "2001-00-31 21:00:00+0");
        TEST_VERIFY(FormatDateTime(DateTime(2001, 1, 1, 0, 0, 0, 3 * 3600).ConvertToTimeZone(-3 * 3600)) == "2001-00-31 18:00:00-10800");

        {
            DateTime date = DateTime::Now();
            TEST_VERIFY(date.ParseISO8601Date("1970-01-01T05:00:00-03:00"));
            TEST_VERIFY(FormatDateTime(date) == "1970-00-01 05:00:00-10800");
        }
        {
            DateTime date = DateTime::Now();
            TEST_VERIFY(date.ParseRFC822Date("Wed, 27 Sep 2006 21:36:45 +0100"));
            TEST_VERIFY(FormatDateTime(date) == "2006-08-27 21:36:45+3600");
        }

        //////////     old strange tests    ////////////////////////////////////////////
        DAVA::DateTime date = DAVA::DateTime::Now();
        Logger::Debug("#Getting of current date:");
        PrintDateTimeContent(date);

        DAVA::DateTime gmDate = DateTime::GmTime(date.GetTimestamp());
        Logger::Debug("#GmTime() for the timestamp of <1st of Feb 2001, 9:00:00> call:");
        PrintDateTimeContent(gmDate);

        DAVA::DateTime localDate = DateTime::LocalTime(date.GetTimestamp());
        Logger::Debug("#LocalTime() for the timestamp of <1st of Feb 2001, 9:00:00> call:");
        PrintDateTimeContent(localDate);

        TEST_VERIFY(true);      // Strange check
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

    String FormatDateTime(const DateTime& dt)
    {
        int32 year = dt.GetYear();
        int32 month = dt.GetMonth();
        int32 day = dt.GetDay();
        int32 hour = dt.GetHour();
        int32 minute = dt.GetMinute();
        int32 sec = dt.GetSecond();
        int32 tz = dt.GetTimeZoneOffset();
        return Format("%04d-%02d-%02d %02d:%02d:%02d+%d", year, month, day, hour, minute, sec, tz);
    }
};
