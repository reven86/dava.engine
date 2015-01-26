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


#include "DXTTest.h"
#include "TextureUtils.h"
#include "Render/Image/ImageSystem.h"
#include "Render/PixelFormatDescriptor.h"

static const PixelFormat formats[] =
{
	FORMAT_DXT1,
	FORMAT_DXT1A,
	FORMAT_DXT3,
	FORMAT_DXT5,
	FORMAT_DXT5NM
};

DXTTest::DXTTest()
: TestTemplate<DXTTest>("DXTTest")
{
    FilePath testFolder = FileSystem::Instance()->GetCurrentDocumentsDirectory() + "DXTTest/";
    FileSystem::Instance()->CreateDirectory(testFolder, true);

    pngSprite = NULL;
    dxtSprite = NULL;
    decompressedPNGSprite = NULL;

    currentTest = FIRST_TEST;
    for(int32 i = 0; i < TESTS_COUNT; ++i)
    {
        const PixelFormatDescriptor & formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(formats[i]);
        RegisterFunction(this, &DXTTest::TestFunction, Format("DXTTest of %s", formatDescriptor.name.c_str()), NULL);
    }
}

void DXTTest::LoadResources()
{
    GetBackground()->SetColor(Color(0.0f, 1.0f, 0.0f, 1.0f));

    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);

    font->SetSize(20);

    compareResultText = new UIStaticText(Rect(0, 256, 512, 200));
    compareResultText->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
    compareResultText->SetMultiline(true);
    compareResultText->SetFont(font);
    compareResultText->SetTextColor(Color::White);
    AddControl(compareResultText);

    SafeRelease(font);
}


void DXTTest::UnloadResources()
{
    RemoveAllControls();

    SafeRelease(compareResultText);

    SafeRelease(pngSprite);
    SafeRelease(dxtSprite);
    SafeRelease(decompressedPNGSprite);
}

void DXTTest::TestFunction(PerfFuncData * data)
{
    DVASSERT(currentTest < TESTS_COUNT);

    if(IsCurrentTestAccepted())
    {
        ReloadSprites();

		float32 differencePersentage = 100.f;
		if (decompressedPNGSprite->GetSize() == dxtSprite->GetSize())
		{
			TextureUtils::CompareResult result = TextureUtils::CompareSprites(decompressedPNGSprite, dxtSprite, formats[currentTest]);
			differencePersentage = ((float32)result.difference / ((float32)result.bytesCount * 256.f)) * 100.f;
		}
        
        data->testData.message = Format("\nDifference: %f%%\nCoincidence: %f%%",
                                        differencePersentage, 100.f - differencePersentage);

        compareResultText->SetText(StringToWString(data->testData.message));
        Logger::Debug(data->testData.message.c_str());

        TEST_VERIFY(differencePersentage < (float32)ACCETABLE_DELTA_IN_PERSENTS);

        //Save images for visual comparision
        Image *firstComparer = TextureUtils::CreateImageAsRGBA8888(decompressedPNGSprite);
        Image *secondComparer = TextureUtils::CreateImageAsRGBA8888(dxtSprite);
        
        FilePath documentsPath = FileSystem::Instance()->GetCurrentDocumentsDirectory();
        
        ImageSystem::Instance()->Save(documentsPath + Format("DXTTest/src_number_%d.png", currentTest), firstComparer);
        ImageSystem::Instance()->Save(documentsPath + Format("DXTTest/dst_number_%d.png", currentTest), secondComparer);
        
        SafeRelease(firstComparer);
        SafeRelease(secondComparer);
    }

    ++currentTest;
}

bool DXTTest::IsCurrentTestAccepted()
{
    RenderManager::Caps deviceCaps = RenderManager::Instance()->GetCaps();

    if(!deviceCaps.isDXTSupported )
    {
        return false;
    }

        
    if((formats[currentTest] == FORMAT_RGBA16161616) && !deviceCaps.isFloat16Supported)
    {
        return false;
    }
    
    if((formats[currentTest] == FORMAT_RGBA32323232) && !deviceCaps.isFloat32Supported)
    {
        return false;
    }


	PixelFormatDescriptor pixelFormat = PixelFormatDescriptor::GetPixelFormatDescriptor(formats[currentTest]);
	
	if (pixelFormat.format == 0)
		return false;
	
	if ((pixelFormat.format == FORMAT_DXT1A || pixelFormat.format == FORMAT_DXT5NM)
		 && (deviceCaps.isDXTSupported && 0 == pixelFormat.internalformat))
	{
		return false;
	}

	if(formats[currentTest] == FORMAT_DXT1A || formats[currentTest] == FORMAT_DXT5NM)
	{
		//not all DXT formats are supported 
		return false;
	}
	

	
	return true;
}


void DXTTest::ReloadSprites()
{
    SafeRelease(pngSprite);
    SafeRelease(dxtSprite);
    SafeRelease(decompressedPNGSprite);

    pngSprite = TextureUtils::CreateSpriteFromTexture(String(Format("~res:/TestData/DXTTest/PNG/number_%d.png", currentTest)));
    dxtSprite = TextureUtils::CreateSpriteFromTexture(String(Format("~res:/TestData/DXTTest/DXT/number_%d.dds", currentTest)));
    decompressedPNGSprite = TextureUtils::CreateSpriteFromTexture(String(Format("~res:/TestData/DXTTest/DecompressedPNG/number_%d.png", currentTest)));
}

void DXTTest::Draw(const DAVA::UIGeometricData &geometricData)
{
    RenderManager::Instance()->ClearWithColor(0.f, 0.0f, 0.f, 1.f);
    
    Sprite::DrawState state;
    state.SetFrame(0);

    if(pngSprite)
    {
        state.SetPosition(0.f, 0.f);
        state.SetScaleSize(256.f, 256.f, pngSprite->GetWidth(), pngSprite->GetHeight());
        RenderSystem2D::Instance()->Draw(pngSprite, &state);
    }

    if(dxtSprite)
    {
        state.SetPosition(260.f, 0.f);
        state.SetScaleSize(256.f, 256.f, dxtSprite->GetWidth(), dxtSprite->GetHeight());
        RenderSystem2D::Instance()->Draw(dxtSprite, &state);
    }

    TestTemplate<DXTTest>::Draw(geometricData);
}


