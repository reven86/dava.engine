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


#include "SaveImageTest.h"
#include "GameCore.h"

using namespace DAVA;

SaveImageTest::SaveImageTest()
: TestTemplate<SaveImageTest>("SaveImageTest")
{
    RegisterFunction(this, &SaveImageTest::PngTest, "PngTest", NULL);
    RegisterFunction(this, &SaveImageTest::JpegTest, "JpegTest", NULL);
}

void SaveImageTest::PngTest(PerfFuncData * data)
{
    Image* img = GetImage();
    FilePath path = FilePath::FilepathInDocuments("testImage.png");
    
    TEST_VERIFY(img->Save(path));
    
    SafeRelease(img);
}

void SaveImageTest::JpegTest(PerfFuncData * data)
{
    Image* img = GetImage();
    FilePath path = FilePath::FilepathInDocuments("testImage.jpeg");
    
    TEST_VERIFY(img->Save(path));
    
    SafeRelease(img);
}

Image* SaveImageTest::GetImage() const
{
    uint32 size = 512;
    Image* img = Image::Create(size, size, FORMAT_RGBA8888);
    uint8* _date = img->data;
    for (uint32 i1 = 0; i1 < size; ++i1)
    {
        for (uint32 i2 = 0; i2 < size; ++i2)
        {
            *_date++ = 0xFF;    // R
            *_date++ = 0x0;    // G
            *_date++ = 0x0;    // B
            *_date++ = 0xFF;    // A
        }
    }
    return img;
}