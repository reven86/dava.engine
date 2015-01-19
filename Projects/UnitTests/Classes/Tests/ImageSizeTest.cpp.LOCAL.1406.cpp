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


#include "ImageSizeTest.h"

ImageSizeTest::ImageSizeTest()
: TestTemplate<ImageSizeTest>("ImageSizeTest")
{
	RegisterFunction(this, &ImageSizeTest::TestFunction, "ImageSizeTest", NULL);
}

void ImageSizeTest::LoadResources()
{
    GetBackground()->SetColor(DAVA::Color(0.0f, 1.0f, 0.0f, 1.0f));
}


void ImageSizeTest::UnloadResources()
{
    RemoveAllControls();
}

void ImageSizeTest::TestFunction(PerfFuncData * data)
{
	static const DAVA::FilePath imagePathnames[DAVA::ImageSystem::FILE_FORMAT_COUNT] = 
	{
		"~res:/TestData/ImageSizeTest/image.png",
		"~res:/TestData/ImageSizeTest/image.jpg",
		"~res:/TestData/ImageSizeTest/image.pvr",
		"~res:/TestData/ImageSizeTest/image.dds"
	};


    //TODO: -1 due to DF-5704
	for(uint32 i = 0; i < DAVA::ImageSystem::FILE_FORMAT_COUNT-1; ++i)
	{
		DAVA::ImageFormatInterface *im = DAVA::ImageSystem::Instance()->GetImageFormatInterface(imagePathnames[i]);	
		Size2i imageSize = im->GetImageSize(imagePathnames[i]);

		TEST_VERIFY(imageSize.dx == 128);
		TEST_VERIFY(imageSize.dy == 128);
	}
}


