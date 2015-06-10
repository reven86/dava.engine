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

DAVA_TESTCLASS(StringFormatTest)
{
    inline void CheckFloatFormat(const WideString& format, float32 value)
    {
        String wideFormatting = WStringToString(Format(format.c_str(), value));
        String narrowFormatting = Format(WStringToString(format).c_str(), value);
        TEST_VERIFY_WITH_MESSAGE(wideFormatting == narrowFormatting, "'" + wideFormatting + " == " + narrowFormatting + "'");
    }

    DAVA_TEST(StringTestFunction)
    {
        WideString formatStr1 = L"%ls %ls";
        WideString value1 = L"test string";
        WideString value2 = L"second";
        TEST_VERIFY(Format(formatStr1.c_str(), value1.c_str(), value2.c_str()) == StringToWString(Format(WStringToString(formatStr1).c_str(), value1.c_str(), value2.c_str() ) ) );
    }

    DAVA_TEST(IntegerTestFunction)
    {
        WideString formatStr1 = L"%i%%";
        WideString formatStr2 = L"%d%%";
        WideString formatStr3 = L"%lld%%";

        int32 value = 1234567890;
        int64 value64 = 1234567890123456789;

        TEST_VERIFY(Format(formatStr1.c_str(), value) == StringToWString(Format(WStringToString(formatStr1).c_str(), value)));
        TEST_VERIFY(Format(formatStr2.c_str(), value) == StringToWString(Format(WStringToString(formatStr2).c_str(), value)));
        TEST_VERIFY(Format(formatStr3.c_str(), value64) == StringToWString(Format(WStringToString(formatStr3).c_str(), value64)));

        value *= -1;
        value64 *= -1;

        TEST_VERIFY(Format(formatStr1.c_str(), value) == StringToWString(Format(WStringToString(formatStr1).c_str(), value)));
        TEST_VERIFY(Format(formatStr2.c_str(), value) == StringToWString(Format(WStringToString(formatStr2).c_str(), value)));
        TEST_VERIFY(Format(formatStr3.c_str(), value64) == StringToWString(Format(WStringToString(formatStr3).c_str(), value64)));
    }

    DAVA_TEST(FloatTestFunction)
    {
        WideString formatStr[] = {
            L"%f",
            L"%6.f", L"%5.f", L"%4.f", L"%3.f", L"%2.f", L"%1.f", L"%0.f",
            L"%.6f", L"%.5f", L"%.4f", L"%.3f", L"%.2f", L"%.1f", L"%.0f",
            L"%6.6f", L"%5.5f", L"%4.4f", L"%3.3f", L"%2.2f", L"%1.1f", L"%0.0f"
        };
        float32 values[] = {
            1234.1234f, 876.876f, 0.1234f, 0.2567f, 0.5f, 0.7543f,
            -1234.1234f, -876.876f, -0.1234f, -0.2567f, -0.5f, -0.7543f
        };
        for (auto& fmt : formatStr)
        {
            for (float32 value : values)
            {
                CheckFloatFormat(fmt.c_str(), value);
            }
        }

        CheckFloatFormat(L"%.3f", 10);
        CheckFloatFormat(L"%.0f", 12980.0f / 1000.0f);
        CheckFloatFormat(L"%.3f", 2.00671148f);
    }

    DAVA_TEST(NarrowStringFormatTest)
    {
        TEST_VERIFY(Format("%%") == "%");
        TEST_VERIFY(Format("%c", 'A') == "A");
        TEST_VERIFY(Format("%lc", L'A') == "A");
        TEST_VERIFY(Format("%s", "this is a test") == "this is a test");
        TEST_VERIFY(Format("%ls", L"this is a test") == "this is a test");

        TEST_VERIFY(Format("%d", 348) == "348");
        TEST_VERIFY(Format("%+d", 348) == "+348");
        TEST_VERIFY(Format("%d", -348) == "-348");
        TEST_VERIFY(Format("%i", 348) == "348");
        TEST_VERIFY(Format("%+i", 348) == "+348");
        TEST_VERIFY(Format("%i", -348) == "-348");

        TEST_VERIFY(Format("%X", 0x1A0D) == "1A0D");
        TEST_VERIFY(Format("%06X", 0x1A0D) == "001A0D");

        TEST_VERIFY(Format("%llX", 0x123456489ABCDEF0) == "123456489ABCDEF0");
    }

    DAVA_TEST(WideStringFormatTest)
    {
        TEST_VERIFY(Format(L"%%") == L"%");
        TEST_VERIFY(Format(L"%c", 'A') == L"A");
        TEST_VERIFY(Format(L"%lc", L'A') == L"A");
        //TEST_VERIFY(Format(L"%s", "this is a test") == L"this is a test");
        TEST_VERIFY(Format(L"%ls", L"this is a test") == L"this is a test");

        TEST_VERIFY(Format(L"%d", 348) == L"348");
        TEST_VERIFY(Format(L"%+d", 348) == L"+348");
        TEST_VERIFY(Format(L"%d", -348) == L"-348");
        TEST_VERIFY(Format(L"%i", 348) == L"348");
        TEST_VERIFY(Format(L"%+i", 348) == L"+348");
        TEST_VERIFY(Format(L"%i", -348) == L"-348");

        TEST_VERIFY(Format(L"%X", 0x1A0D) == L"1A0D");
        TEST_VERIFY(Format(L"%06X", 0x1A0D) == L"001A0D");

        TEST_VERIFY(Format(L"%llX", 0x123456489ABCDEF0) == L"123456489ABCDEF0");
    }

    DAVA_TEST(VeryLongStringFormatTest)
    {
        TEST_VERIFY(Format("%s%s%s", String(100, 'A').c_str(), String(200, 'B').c_str(), String(400, 'C').c_str()).length() == 700);
        TEST_VERIFY(Format("%ls%ls%ls", WideString(100, 'A').c_str(), WideString(200, 'B').c_str(), WideString(400, 'C').c_str()).length() == 700);
        //TEST_VERIFY(Format(L"%hs%hs%hs", String(100, 'A').c_str(), String(200, 'B').c_str(), String(400, 'C').c_str()).length() == 700);
        TEST_VERIFY(Format(L"%ls%ls%ls", WideString(100, 'A').c_str(), WideString(200, 'B').c_str(), WideString(400, 'C').c_str()).length() == 700);
    }
};
