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



#include "AlignTest.h"
#include "TextureUtils.h"
#include "Render/RenderManager.h"

const float32 ACCETABLE_DELTA_IN_PERSENTS = 2.0f;

#if defined(__DAVAENGINE_MACOS__)
const char* REFERENCE_IMAGE_PATH = "~res:/TestData/AlignTest/MacOS/test%d.png";
#elif defined(__DAVAENGINE_WIN32__)
const char* REFERENCE_IMAGE_PATH = "~res:/TestData/AlignTest/Win32/test%d.png";
#elif defined(__DAVAENGINE_IPHONE__)
const char* REFERENCE_IMAGE_PATH = "~res:/TestData/AlignTest/IOS/test%d.png";
#elif defined(__DAVAENGINE_ANDROID__)
const char* REFERENCE_IMAGE_PATH = "~res:/TestData/AlignTest/Android/test%d.png";
#endif

const WideString controlText = L"THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED";

const int AlignTest::alignTypesData[] =
{
    ALIGN_LEFT | ALIGN_TOP,
    ALIGN_LEFT | ALIGN_VCENTER,
    ALIGN_LEFT | ALIGN_BOTTOM,
	ALIGN_HCENTER | ALIGN_TOP,
    ALIGN_HCENTER | ALIGN_VCENTER,
    ALIGN_HCENTER | ALIGN_BOTTOM,
    ALIGN_RIGHT | ALIGN_TOP,
    ALIGN_RIGHT | ALIGN_VCENTER,
    ALIGN_RIGHT | ALIGN_BOTTOM,    
    ALIGN_HJUSTIFY
};

AlignTest::AlignTest():
TestTemplate<AlignTest>("AlignTest"),
	currentAlignIndex(0),
	currenTestIndex(0),
	data(NULL)
{	
	
	RegisterFunction(this, &AlignTest::MultilineEnable, Format("MultilineTest"), NULL);
	RegisterFunction(this, &AlignTest::ResizeControl, Format("ResizeTest"), NULL);
	RegisterFunction(this, &AlignTest::MoveControl, Format("MoveTest"), NULL);
	// Register align function for each align option
	for (int32 i = 0; i < GetAlignTypesCount(); ++i)
	{
		RegisterFunction(this, &AlignTest::AlignText, Format("AlignTest"), NULL);
	}
}

void AlignTest::LoadResources()
{
	// DF-1627 - Always set black background for this test for Windows - all screenshots should be the same
#ifdef __DAVAENGINE_WIN32__
	GetBackground()->SetColor(Color::Black);
	GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
#endif

    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");		
    DVASSERT(font);

    staticText = new UIStaticText();
    staticText->SetRect(Rect(10.f, 10.f, 400.f, 200.f));
	staticText->SetTextColor(Color::White);
	staticText->SetDebugDraw(true);    
    staticText->SetFont(font);
	staticText->SetText(controlText);
	AddControl(staticText);	

    staticText2 = new UIStaticText();
    staticText2->SetRect(Rect(550.f, 10.f, 200.f, 100.f));
	staticText2->SetTextColor(Color::White);
	staticText2->SetDebugDraw(true);
    staticText2->SetFont(font);
	staticText2->SetText(controlText);
	AddControl(staticText2);

	SafeRelease(font);
}

void AlignTest::UnloadResources()
{
	RemoveAllControls();
    SafeRelease(staticText);
    SafeRelease(staticText2);
}

void AlignTest::MultilineEnable(PerfFuncData * testData)
{
	staticText->SetMultiline(true);
   	staticText2->SetMultiline(true);
	MakeScreenShot();
	data = testData;
}

void AlignTest::ResizeControl(PerfFuncData * testData)
{
	Rect rect = staticText->GetRect();
	rect.dx = 500.f;
	rect.dy = 100.f;
	staticText->SetRect(rect);
		
	rect = staticText2->GetRect();
	rect.dx = 300.f;
	rect.dy = 150.f;
	staticText2->SetRect(rect);
	
	MakeScreenShot();
	data = testData;
}

void AlignTest::MoveControl(PerfFuncData * testData)
{
	Rect rect = staticText->GetRect();
	rect.x = 50.f;
	rect.y = 200.f;
	staticText->SetRect(rect);
		
	rect = staticText2->GetRect();
	rect.x = 10.f;
	staticText2->SetRect(rect);
	MakeScreenShot();
	data = testData;
}

void AlignTest::AlignText(PerfFuncData * testData)
{
	staticText->SetTextAlign(GetAlignType(currentAlignIndex));
	staticText2->SetTextAlign(GetAlignType(currentAlignIndex));
		
	currentAlignIndex++;
	MakeScreenShot();
	data = testData;
}

int AlignTest::GetAlignTypesCount()
{
	return sizeof(alignTypesData)/sizeof(*alignTypesData);
}

int AlignTest::GetAlignType(int index)
{
    return alignTypesData[index];
}

void AlignTest::MakeScreenShot()
{
	RenderManager::Instance()->RequestGLScreenShot(this);
	currenTestIndex++;
}

void AlignTest::OnScreenShot(Image *testImage)
{
	//Use this code to generate new reference screenshots
//	FilePath workingPath = FileSystem::Instance()->GetCurrentWorkingDirectory();
//	ImageLoader::Save(testImage, workingPath + Format("Data/test%d.png", currenTestIndex));
	VerifyTestImage(testImage);
}

void AlignTest::VerifyTestImage(Image *testImage)
{
	// Loade reference image for current test
	Image *referenceImage = NULL;
	Vector<Image *> imageSet;
    ImageSystem::Instance()->Load(Format(REFERENCE_IMAGE_PATH, currenTestIndex), imageSet);
	if(imageSet.size() != 0)
    {
		referenceImage = imageSet[0];
	}
	// Compare reference and test images and calculate differebce persentage
	float32 differencePersentage = 100.f;
	if (testImage && referenceImage)
	{

        // Temporary fix
        // Different devices have different screen resolution, but reference images are in only one resolution.
        // This causes asserts and sometimes crashes in the image compare routine.
        uint32 pixelsCount1 = testImage->GetWidth() * testImage->GetHeight();
        uint32 pixelsCount2 = referenceImage->GetWidth() * referenceImage->GetHeight();

        if (pixelsCount1 > pixelsCount2)
        {
            testImage->ResizeImage(referenceImage->GetWidth(), referenceImage->GetHeight());
        }
        else if (pixelsCount1 < pixelsCount2)
        {
            referenceImage->ResizeImage(testImage->GetWidth(), testImage->GetHeight());
        }
        // end of temporary fix

		TextureUtils::CompareResult result = TextureUtils::CompareImages(testImage,
																	referenceImage, FORMAT_RGBA8888);
				
		differencePersentage = ((float32)result.difference / ((float32)result.bytesCount * 256.f)) * 100.f;
	}
	// Verify compare results
	if (data)
	{
    	TEST_VERIFY(differencePersentage < ACCETABLE_DELTA_IN_PERSENTS);
	}

}