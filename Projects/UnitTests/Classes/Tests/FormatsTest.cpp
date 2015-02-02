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


FormatsTest::FormatsTest()
: TestTemplate<FormatsTest>("FormatsTest")
{
    RegisterFunction(this, &FormatsTest::TestFunction, "FormatsTest", NULL);
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
#if defined (__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
    
    for(uint32 f = FORMAT_RGBA8888; f < FORMAT_COUNT; ++f)
    {
        if(FORMAT_REMOVED_DXT_1N == f)
        {
            continue;
        }
        
        if(FORMAT_PVR2_2 <= f)
        {
            //not implemented reading in dava.framework
            continue;
        }
        
        const DAVA::PixelFormat requestedFormat = (const DAVA::PixelFormat)f;
        const String formatName = GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(requestedFormat);
        
        data->testData.message = Format("\nFormatsTest: \n\tformat = %s\n", formatName.c_str());

        
        const DAVA::FilePath pngPathname(DAVA::Format("~res:/TestData/FormatTest/%s.png", formatName.c_str()));
        const DAVA::FilePath compressedPathname = DAVA::FilePath::CreateWithNewExtension(pngPathname, ".dat");
     
        DAVA::Vector<DAVA::Image *> pngImages, compressedImages;
        const DAVA::eErrorCode loadPng = DAVA::ImageSystem::Instance()->Load(pngPathname, pngImages);
        TEST_VERIFY(DAVA::SUCCESS == loadPng);

        const DAVA::eErrorCode loadCompressed = DAVA::ImageSystem::Instance()->Load(compressedPathname, compressedImages);
        TEST_VERIFY(DAVA::SUCCESS == loadCompressed);

        if(pngImages.empty() || compressedImages.empty())
        {
            TEST_VERIFY(false);
        }
        else
        {
            const DAVA::PixelFormat comparedFormat = ((DAVA::FORMAT_A8 == requestedFormat) || (DAVA::FORMAT_A16 == requestedFormat))
                                                        ?   (const DAVA::PixelFormat)requestedFormat
                                                        :   DAVA::FORMAT_RGBA8888;
            
            const TextureUtils::CompareResult cmpRes = TextureUtils::CompareImages(pngImages[0], compressedImages[0], comparedFormat);
            

            float32 differencePersentage = ((float32)cmpRes.difference / ((float32)cmpRes.bytesCount * 256.f)) * 100.f;
            
            data->testData.message += Format("\tDifference: %f%%\n\tCoincidence: %f%%", differencePersentage, 100.f - differencePersentage);

            TEST_VERIFY(differencePersentage <= FormatsTest::MAX_DIFFERENCE);
        }
    }
    
#endif //#if defined (__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)

}
