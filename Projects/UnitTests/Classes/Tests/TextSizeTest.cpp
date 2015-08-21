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

struct TestStruct
{
    String description;
    WideString testString;
};

static const float32 TEST_ACCURACY = 2.f;

DAVA_TESTCLASS(TextSizeTest)
{
    Font* font = nullptr;

    TextSizeTest()
    {
        font = FTFont::Create("~res:/Fonts/arialuni.ttf");
        DVASSERT(font);
        font->SetSize(20);
    }

    ~TextSizeTest()
    {
        SafeRelease(font);
    }

    DAVA_TEST(TestFunction)
    {
        Array<TestStruct, 5> testData = { {
            { "English", L"THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED" },
            { "Arabic", L"هذا البرنامج يتم توفيرها من قبل DAVA، INC والمساهمين 'كما هو' وأية ضمانات صريحة أو ضمنية، بما في ذلك على سبيل المثال لا الحصر، ضمني" },
            { "Japanese", L"本ソフトウェアはDAVA、株式会社によって提供され、協力者「現状のまま」と明示しまたはその他を含む保証を、黙示、これらに限定されないが、暗黙的に指定され" },
            { "Chinese", L"本軟件提供的DAVA，公司和貢獻者“按原樣”提供任何明示或暗示的擔保，包括但不限於，暗示" },
            { "Hindi", L"इस सॉफ़्टवेयर Dava, कांग्रेस द्वारा प्रदत्त योगदानकर्ताओं 'जैसी है' और किसी प्रकट या वारंटियों, निहित है, लेकिन सीमित नहीं, निहित है" }
        } };

        for (size_t k = 0;k < testData.size();++k)
        {
            const WideString& testString = testData[k].testString;
            Vector<float32> charSizes;
            Size2i size = font->GetStringSize(testString, &charSizes);
            float32 charsSum = 0;
            for (uint32 i = 0; i < charSizes.size(); ++i)
            {
                charsSum += charSizes[i];
            }

            charsSum = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtualX(charsSum);
            TEST_VERIFY_WITH_MESSAGE(fabsf(size.dx - charsSum) < TEST_ACCURACY, testData[k].description);
        }
    }
};
