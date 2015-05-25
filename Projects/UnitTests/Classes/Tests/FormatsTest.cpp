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


#include "FormatsTest.h"
#include "Render/PixelFormatDescriptor.h"
#include "Infrastructure/TextureUtils.h"

const DAVA::float32 FormatsTest::MAX_DIFFERENCE = 2.f; // in percents

FormatsTest::FormatsTest()
    : TestTemplate<FormatsTest>("FormatsTest")
{
    RegisterFunction(this, &FormatsTest::TestFunction, "FormatsTest", nullptr);
}

void FormatsTest::LoadResources()
{
    GetBackground()->SetColor(DAVA::Color(0.0f, 1.0f, 0.0f, 1.0f));
}

void FormatsTest::UnloadResources()
{
    RemoveAllControls();
}

void FormatsTest::TestFunction(PerfFuncData * data)
{
    TestJpeg(data);
    TestPng(data);
    TestPvr(data);
    TestDds(data);
}

void FormatsTest::TestJpeg(PerfFuncData *data)
{
    Vector<PixelFormat> suportedFormats;
    suportedFormats.push_back(FORMAT_A8);
    suportedFormats.push_back(FORMAT_RGB888);

    for (PixelFormat requestedFormat : suportedFormats)
    {
        const String formatName = GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(requestedFormat);

        data->testData.message = Format("\nFormatsTest_JPEG: \n\tformat = %s\n", formatName.c_str());

        const DAVA::FilePath compressedPathname(DAVA::Format("~res:/TestData/FormatsTest/jpeg/%s.dat", formatName.c_str()));

        TestImageInfo(compressedPathname, requestedFormat, data);
    }
}

void FormatsTest::TestPng(PerfFuncData *data)
{
    Vector<PixelFormat> suportedFormats;
    suportedFormats.push_back(FORMAT_A8);
    suportedFormats.push_back(FORMAT_A16);
    suportedFormats.push_back(FORMAT_RGBA8888);

    for (PixelFormat requestedFormat : suportedFormats)
    {
        const String formatName = GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(requestedFormat);

        data->testData.message = Format("\nFormatsTest_PNG: \n\tformat = %s\n", formatName.c_str());

        const DAVA::FilePath compressedPathname(DAVA::Format("~res:/TestData/FormatsTest/png/%s.dat", formatName.c_str()));

        TestImageInfo(compressedPathname, requestedFormat, data);
    }
}

void FormatsTest::TestPvr(PerfFuncData *data)
{
    Vector<PixelFormat> suportedFormats;
    suportedFormats.push_back(FORMAT_RGBA8888);
    suportedFormats.push_back(FORMAT_RGBA5551);
    suportedFormats.push_back(FORMAT_RGBA4444);
    suportedFormats.push_back(FORMAT_RGB888);
    suportedFormats.push_back(FORMAT_RGB565);
    suportedFormats.push_back(FORMAT_A8);
    suportedFormats.push_back(FORMAT_PVR2);
    suportedFormats.push_back(FORMAT_PVR4);
    suportedFormats.push_back(FORMAT_ETC1);

    for (PixelFormat requestedFormat : suportedFormats)
    {
        const String formatName = GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(requestedFormat);

        data->testData.message = Format("\nFormatsTest_PVR: \n\tformat = %s\n", formatName.c_str());

        const DAVA::FilePath pngPathname(DAVA::Format("~res:/TestData/FormatsTest/pvr/%s.png", formatName.c_str()));
        const DAVA::FilePath compressedPathname = DAVA::FilePath::CreateWithNewExtension(pngPathname, ".dat");

        TestImageInfo(compressedPathname, requestedFormat, data);

#if !(defined (__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WINDOWS__))
        continue;
#endif //#if !(defined (__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WINDOWS__))

        const DAVA::PixelFormatDescriptor & descriptor = DAVA::PixelFormatDescriptor::GetPixelFormatDescriptor(requestedFormat);
        if (descriptor.isHardwareSupported)
            continue;

        DAVA::Vector<DAVA::Image *> pngImages;
        DAVA::Vector<DAVA::Image *> compressedImages;
        const DAVA::eErrorCode loadPng = DAVA::ImageSystem::Instance()->Load(pngPathname, pngImages);
        TEST_VERIFY(DAVA::SUCCESS == loadPng);

        const DAVA::eErrorCode loadCompressed = DAVA::ImageSystem::Instance()->Load(compressedPathname, compressedImages);
        TEST_VERIFY(DAVA::SUCCESS == loadCompressed);

        if (pngImages.empty() || compressedImages.empty())
        {
            TEST_VERIFY(false);
        }
        else
        {
            const DAVA::PixelFormat comparedFormat = ((DAVA::FORMAT_A8 == requestedFormat) || (DAVA::FORMAT_A16 == requestedFormat))
                                                      ? (const DAVA::PixelFormat)requestedFormat
                                                      : DAVA::FORMAT_RGBA8888;

            const TextureUtils::CompareResult cmpRes = TextureUtils::CompareImages(pngImages[0], compressedImages[0], comparedFormat);

            float32 differencePersentage = ((float32)cmpRes.difference / ((float32)cmpRes.bytesCount * 256.f)) * 100.f;

            data->testData.message += Format("\tDifference: %f%%\n\tCoincidence: %f%%", differencePersentage, 100.f - differencePersentage);

            TEST_VERIFY(differencePersentage <= FormatsTest::MAX_DIFFERENCE);
        }
    }
}

void FormatsTest::TestDds(PerfFuncData *data)
{
    Vector<PixelFormat> suportedFormats;
    suportedFormats.push_back(FORMAT_DXT1);
    suportedFormats.push_back(FORMAT_DXT1A);
    suportedFormats.push_back(FORMAT_DXT3);
    suportedFormats.push_back(FORMAT_DXT5);
    suportedFormats.push_back(FORMAT_DXT5NM);
    suportedFormats.push_back(FORMAT_ATC_RGB);
    suportedFormats.push_back(FORMAT_ATC_RGBA_EXPLICIT_ALPHA);
    suportedFormats.push_back(FORMAT_ATC_RGBA_INTERPOLATED_ALPHA);

    for (PixelFormat requestedFormat : suportedFormats)
    {
        const String formatName = GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(requestedFormat);

        data->testData.message = Format("\nFormatsTest_DDS: \n\tformat = %s\n", formatName.c_str());

        const DAVA::FilePath pngPathname(DAVA::Format("~res:/TestData/FormatsTest/dds/%s.png", formatName.c_str()));
        const DAVA::FilePath compressedPathname = DAVA::FilePath::CreateWithNewExtension(pngPathname, ".dat");

        TestImageInfo(compressedPathname, requestedFormat, data);

#if !(defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WINDOWS__))
        continue;
#endif //#if !(defined (__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WINDOWS__))

#if defined(__DAVAENGINE_MACOS__) && (requestedFormat == FORMAT_ATC_RGBA_INTERPOLATED_ALPHA)
        continue;
#endif //#if defined (__DAVAENGINE_MACOS__)

        const DAVA::PixelFormatDescriptor & descriptor = DAVA::PixelFormatDescriptor::GetPixelFormatDescriptor(requestedFormat);
        if (descriptor.isHardwareSupported)
            continue;

        DAVA::Vector<DAVA::Image *> pngImages;
        DAVA::Vector<DAVA::Image *> compressedImages;
        const DAVA::eErrorCode loadPng = DAVA::ImageSystem::Instance()->Load(pngPathname, pngImages);
        TEST_VERIFY(DAVA::SUCCESS == loadPng);

        const DAVA::eErrorCode loadCompressed = DAVA::ImageSystem::Instance()->Load(compressedPathname, compressedImages);
        TEST_VERIFY(DAVA::SUCCESS == loadCompressed);

        if (pngImages.empty() || compressedImages.empty())
        {
            TEST_VERIFY(false);
        }
        else
        {
            const DAVA::PixelFormat comparedFormat = ((DAVA::FORMAT_A8 == requestedFormat) || (DAVA::FORMAT_A16 == requestedFormat))
                                                      ? (const DAVA::PixelFormat)requestedFormat
                                                      : DAVA::FORMAT_RGBA8888;

            const TextureUtils::CompareResult cmpRes = TextureUtils::CompareImages(pngImages[0], compressedImages[0], comparedFormat);

            float32 differencePersentage = ((float32)cmpRes.difference / ((float32)cmpRes.bytesCount * 256.f)) * 100.f;

            data->testData.message += Format("\tDifference: %f%%\n\tCoincidence: %f%%", differencePersentage, 100.f - differencePersentage);

            TEST_VERIFY(differencePersentage <= FormatsTest::MAX_DIFFERENCE);
        }
    }
}

void FormatsTest::TestImageInfo(const DAVA::FilePath &fileName, DAVA::PixelFormat &requestedFormat, PerfFuncData *data)
{
    // NOTE: if file is generated in DXT1A format then lib returned new file in DXT1
    switch (requestedFormat)
    {
        case FORMAT_DXT1A:
            requestedFormat = FORMAT_DXT1;
            break;
        case FORMAT_DXT5NM:
            requestedFormat = FORMAT_DXT5;
            break;
        default:
            break;
    }

    ImageInfo info = ImageSystem::Instance()->GetImageInfo(fileName);
    TEST_VERIFY(info.format == requestedFormat);
    TEST_VERIFY(info.width == 256);
    TEST_VERIFY(info.height == 256);
}
