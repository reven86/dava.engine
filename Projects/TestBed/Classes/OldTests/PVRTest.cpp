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


#include "PVRTest.h"
#include "TextureUtils.h"
#include "Render/PixelFormatDescriptor.h"

static const PixelFormat formats[] =
{
    FORMAT_PVR2,
    FORMAT_PVR4,
    FORMAT_RGBA8888,
    FORMAT_RGBA4444,
    FORMAT_RGBA5551,
    FORMAT_RGB888,
    FORMAT_RGB565,
    FORMAT_A8,
    FORMAT_RGBA16161616,
    FORMAT_RGBA32323232
};


PVRTest::PVRTest()
: TestTemplate<PVRTest>("PVRTest")
{
    FilePath testFolder = FileSystem::Instance()->GetCurrentDocumentsDirectory() + "PVRTest/";
    FileSystem::Instance()->CreateDirectory(testFolder, true);

    pngSprite = NULL;
    pvrSprite = NULL;
    decompressedPNGSprite = NULL;

    currentTest = FIRST_TEST;
    for(int32 i = 0; i < TESTS_COUNT; ++i)
    {
        const PixelFormatDescriptor & formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(formats[i]);
        RegisterFunction(this, &PVRTest::TestFunction, Format("PVRTest of %s", formatDescriptor.name.c_str()), NULL);
    }
}

void PVRTest::LoadResources()
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


void PVRTest::UnloadResources()
{
    RemoveAllControls();
    
    SafeRelease(compareResultText);
    
    SafeRelease(pngSprite);
    SafeRelease(pvrSprite);
    SafeRelease(decompressedPNGSprite);
}

void PVRTest::TestFunction(PerfFuncData * data)
{
    DVASSERT(currentTest < TESTS_COUNT);
    
    if(IsCurrentTestAccepted())
    {
        ReloadSprites();
        
        TextureUtils::CompareResult result = TextureUtils::CompareSprites(decompressedPNGSprite, pvrSprite, formats[currentTest]);
        float32 differencePersentage = ((float32)result.difference / ((float32)result.bytesCount * 256.f)) * 100.f;
        
        data->testData.message = Format("\nDifference: %f%%\nCoincidence: %f%%", differencePersentage, 100.f - differencePersentage);
        
        compareResultText->SetText(StringToWString(data->testData.message));
        Logger::Debug(data->testData.message.c_str());
        
        TEST_VERIFY(differencePersentage < (float32)ACCETABLE_DELTA_IN_PERSENTS);
        
        
        //Save images for visual comparision
        Image *firstComparer = TextureUtils::CreateImageAsRGBA8888(decompressedPNGSprite);
        Image *secondComparer = TextureUtils::CreateImageAsRGBA8888(pvrSprite);
        
        FilePath documentsPath = FileSystem::Instance()->GetCurrentDocumentsDirectory();
        ImageSystem::Instance()->Save(documentsPath + Format("PVRTest/src_number_%d.png", currentTest), firstComparer);
        ImageSystem::Instance()->Save(documentsPath + Format("PVRTest/dst_number_%d.png", currentTest), secondComparer);
        SafeRelease(firstComparer);
        SafeRelease(secondComparer);
    }
    
    ++currentTest;
}

bool PVRTest::IsCurrentTestAccepted()
{
    RenderManager::Caps deviceCaps = RenderManager::Instance()->GetCaps();
    
#if defined (__DAVAENGINE_ANDROID__)
    if((formats[currentTest] == FORMAT_PVR2) && !deviceCaps.isPVRTCSupported)
    {
        return false;
    }
    if((formats[currentTest] == FORMAT_PVR4) && !deviceCaps.isPVRTCSupported)
    {
        return false;
    }
    
#endif //#if defined (__DAVAENGINE_ANDROID__)
    if((formats[currentTest] == FORMAT_RGBA16161616) && !deviceCaps.isFloat16Supported)
    {
        return false;
    }
    
    if((formats[currentTest] == FORMAT_RGBA32323232) && !deviceCaps.isFloat32Supported)
    {
        return false;
    }

    return true;
}


void PVRTest::ReloadSprites()
{
    SafeRelease(pngSprite);
    SafeRelease(pvrSprite);
    SafeRelease(decompressedPNGSprite);
    
    pngSprite = TextureUtils::CreateSpriteFromTexture(String(Format("~res:/TestData/PVRTest/PNG/number_%d.png", currentTest)));
    pvrSprite = TextureUtils::CreateSpriteFromTexture(String(Format("~res:/TestData/PVRTest/PVR/number_%d.pvr", currentTest)));
    decompressedPNGSprite = TextureUtils::CreateSpriteFromTexture(String(Format("~res:/TestData/PVRTest/DecompressedPNG/number_%d.png", currentTest)));
}

void PVRTest::Draw(const DAVA::UIGeometricData &geometricData)
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
    
    if(pvrSprite)
    {
        state.SetPosition(260.f, 0.f);
        state.SetScaleSize(256.f, 256.f, pvrSprite->GetWidth(), pvrSprite->GetHeight());
        RenderSystem2D::Instance()->Draw(pvrSprite, &state);
    }
    
    TestTemplate<PVRTest>::Draw(geometricData);
}


