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


#include "RectSpriteTest.h"
#include "FileSystem/File.h"

static const float RECT_SPRITE_TEST_AUTO_CLOSE_TIME = 30.0f;

RectSpriteTest::RectSpriteTest()
:	TestTemplate<RectSpriteTest>("RectSpriteTest")
,   testFinished(false)
{
	RegisterFunction(this, &RectSpriteTest::TestFunction, Format("RectSpriteTest test"), NULL);
}

void RectSpriteTest::LoadResources()
{
	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(20);

	testButton = new UIButton(Rect(0, 0, 300, 30));
	testButton->SetStateFont(0xFF, font);
	testButton->SetStateText(0xFF, L"Finish Test");
	testButton->SetStateFontColor(0xFF, Color::White);
	testButton->SetDebugDraw(true);
	testButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &RectSpriteTest::ButtonPressed));
	AddControl(testButton);

    AddImage("~res:/TestData/RectSpriteTest/1.png");
    AddImage("~res:/TestData/RectSpriteTest/2.png");
    AddImage("~res:/TestData/RectSpriteTest/3.png");
    AddImage("~res:/TestData/RectSpriteTest/4.png");

    sprites.push_back(Sprite::CreateFromSourceFile("~res:/TestData/RectSpriteTest/1.png"));
    sprites.push_back(Sprite::CreateFromSourceFile("~res:/TestData/RectSpriteTest/2.png"));
    sprites.push_back(LoadSpriteFromFileBuf("~res:/TestData/RectSpriteTest/3.png"));
    sprites.push_back(LoadSpriteFromFileBuf("~res:/TestData/RectSpriteTest/4.png"));
}

void RectSpriteTest::AddImage(const FilePath& filePath)
{
    Vector<Image*> imageSet;
    ImageSystem::Instance()->Load(filePath, imageSet);
    if (imageSet.size() == 0)
    {
        images.push_back(NULL);
    }
    else
    {
        images.push_back(imageSet[0]);
    }
}

Sprite* RectSpriteTest::LoadSpriteFromFileBuf(const FilePath& filePath)
{
    File* file = File::Create(filePath, File::OPEN | File::READ);
    if (file == NULL)
    {
        return NULL;
    }

    uint8* buf = new uint8[file->GetSize()];
    file->Read(buf, file->GetSize());

    Sprite* sprite = Sprite::CreateFromSourceData(buf, file->GetSize());

    SafeRelease(file);
    SafeDeleteArray(buf);

    return sprite;
}

void RectSpriteTest::UnloadResources()
{
    RemoveAllControls();

    for_each(images.begin(), images.end(), SafeRelease<Image>);
    for_each(sprites.begin(), sprites.end(), SafeRelease<Sprite>);
}

bool RectSpriteTest::RunTest(int32 testNum)
{
	TestTemplate<RectSpriteTest>::RunTest(testNum);
	return testFinished;
}

void RectSpriteTest::Draw(const DAVA::UIGeometricData &geometricData)
{
    RenderManager::Instance()->ClearWithColor(0.3f, 0.0f, 0.f, 1.f);

    Sprite::DrawState state;
    state.SetFrame(0);

    if (sprites[1])
    {
        state.SetPosition(70.f, 40.f);
        state.SetAngle(0.1f);
        RenderSystem2D::Instance()->Draw(sprites[1], &state);
    }
    if (sprites[2])
    {
        state.SetPosition(200.f, 50.f);
        state.SetAngle(0.785398f);
        RenderSystem2D::Instance()->Draw(sprites[2], &state);
    }
    if (sprites[3])
    {
        state.SetPosition(110.f, 60.f);
        state.SetAngle(0.f);
        RenderSystem2D::Instance()->Draw(sprites[3], &state);
    }
    if (sprites[0])
    {
        state.SetPosition(300.f, 200.f);
        state.SetAngle(-0.2f);
        RenderSystem2D::Instance()->Draw(sprites[0], &state);
    }

    TestTemplate<RectSpriteTest>::Draw(geometricData);
}

void RectSpriteTest::TestFunction(TestTemplate<RectSpriteTest>::PerfFuncData *data)
{
    if (!testFinished)
    {
        return;
    }

    bool err = false;
    for (uint32 i = 0; i < sprites.size(); ++i)
    {
        if (images[i] == NULL)
        {
            err = true;

            String s = Format("Error: image %d.png was not loaded", i + 1);
            LogError(data, s);
            Logger::Error(s.c_str());
        }
        if (sprites[i] == NULL)
        {
            err = true;

            String s = Format("Error: sprite from image %d.png was not loaded", i + 1);
            LogError(data, s);
            Logger::Error(s.c_str());
        }

        if (images[i] && sprites[i])
        {
            Vector2 spriteSize = sprites[i]->GetSize();
            Vector2 imageSize = Vector2(static_cast<float32>(images[i]->GetWidth()), 
				static_cast<float32>(images[i]->GetHeight()));

            if (spriteSize != imageSize)
            {
                err = true;

                String s = Format("Error: sprite and image sizes are different; %i.png", i + 1);
                LogError(data, s);
                Logger::Error(s.c_str());
            }
        }
    }

    TEST_VERIFY(!err);
}

void RectSpriteTest::DidAppear()
{
	onScreenTime = 0.f;
}

void RectSpriteTest::Update(float32 timeElapsed)
{
	onScreenTime += timeElapsed;
	if(onScreenTime > RECT_SPRITE_TEST_AUTO_CLOSE_TIME)
	{
		testFinished = true;
	}
    
	TestTemplate<RectSpriteTest>::Update(timeElapsed);
}

void RectSpriteTest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
	testFinished = true;
}
