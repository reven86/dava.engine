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


#include "ResultScreen.h"
#include "SettingsManager.h"
#include "GameCore.h"
#include "Config.h"

using namespace DAVA;

ResultScreen::ResultScreen(const LandscapeTestData& testData, const FilePath& filename, Texture* landscapeTexture)
:	isFinished(false),
	state(RESULT_STATE_NORMAL),
	testData(testData)
{
	this->filename = filename;
	
	texture = SafeRetain(landscapeTexture);
	textureSprite = NULL;
	resultSprite = NULL;
}

ResultScreen::~ResultScreen()
{
    SafeRelease(fileNameText);
    SafeRelease(statText[0]);
    SafeRelease(statText[1]);
    SafeRelease(statText[2]);
    SafeRelease(tapToContinue);
    SafeRelease(screenshotText);
	SafeRelease(textureSprite);
	SafeRelease(resultSprite);
	SafeRelease(texture);
}

void ResultScreen::LoadResources()
{
	Vector2 spriteSize((float32)texture->GetWidth(), (float32)texture->GetHeight());
	textureSprite = Sprite::CreateFromTexture(texture, 0, 0, spriteSize.x, spriteSize.y);
	resultSprite = Sprite::CreateAsRenderTarget(spriteSize.x * RESULT_TEXTURE_SCALE,
                                                spriteSize.y * RESULT_TEXTURE_SCALE,
                                                FORMAT_RGBA8888, true);
}

void ResultScreen::UnloadResources()
{
	RemoveAllControls();
}

void ResultScreen::WillAppear()
{
    PrepareSprite();
}

void ResultScreen::WillDisappear()
{
}

void ResultScreen::SaveResults()
{
    Core *core=DAVA::Core::Instance();
    Vector2 screenSize(core->GetVirtualScreenWidth(), core->GetVirtualScreenHeight());
    
    Image* image = resultSprite->GetTexture()->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);
    FilePath saveFileName = FileSystem::Instance()->GetUserDocumentsPath();
    saveFileName += filename.GetFilename() + ".png";
    ImageLoader::Save(image, saveFileName);
    
    Map<String, String> results;
	Map<String, Texture *> textures;
	results["TextureMemorySize"] = Format("%.2f Mb", testData.GetTextureMemorySize()/(1024.f * 1024.f));
	results["TextureFilesSize"] = Format("%.2f Mb", testData.GetTexturesFilesSize()/(1024.f * 1024.f));

 
    float32 avFps = 0.f;
    int32 count = testData.GetItemCount();
	results["FPS items count"] = Format("%d", count);
    for(int32 i = 0; i < count; ++i)
    {
        const FpsStatItem& item = testData.GetItem(i);
        
        float32 avPerItem = 0.f;
        
        for(int32 fps = 0; fps < SECTORS_COUNT; ++fps)
        {
            avPerItem += item.avFps[fps];
        }
        
        avPerItem /= (float32)SECTORS_COUNT;
        
        results[Format("FPS average fps per %d item", i)] = Format("%f fps", avPerItem);
        
        avFps += avPerItem;
    }
    
    if(count)
    {
        results["FPS average value"] = Format("%f fps", avFps / (float32)count);
    }
    else
    {
        results["FPS average value"] = "0 fps";
    }
    
    
    
	String filePath = testData.GetSceneFilePath().GetAbsolutePathname();
    results["SceneFilePath"] = filePath.substr(filePath.find("Maps"));
    
	FilePath folderPathname("~doc:/PerformanceTestResult/");
    FileSystem::Instance()->CreateDirectory(folderPathname);
    FilePath statFileName = folderPathname + FilePath::CreateWithNewExtension(filename, ".txt").GetFilename();
    File* file = File::Create(statFileName, File::CREATE | File::WRITE);
    if (file)
    {
        Map<String, String>::const_iterator it = results.begin();
        for(; it != results.end(); it++)
        {
            // "Format" sometimes doesn't work correct with strings on some platforms
            file->WriteLine(((*it).first + ": " + (*it).second).c_str());
        }
        
        SafeRelease(file);
    }
    
    FilePath levelName = FilePath::CreateWithNewExtension(filename, "").GetFilename();

    if(!GameCore::Instance()->FlushToDB(levelName, results, saveFileName))
        Logger::Debug("Error sending data to DB (connection is lost) !!!");
    
    state = RESULT_STATE_FINISHED;
}

void ResultScreen::Input(UIEvent * event)
{
	if(event->phase == UIEvent::PHASE_BEGAN)
    {
		if(!isFinished && state == RESULT_STATE_NORMAL)
        {
            if(resultSprite != 0)
            {
                SaveResults();
            }
		}
	}
}

void ResultScreen::Update(float32 timeElapsed)
{
	UIScreen::Update(timeElapsed);
	
    if(!isFinished && state == RESULT_STATE_NORMAL && resultSprite != 0)
    {
        SaveResults();
    }
    
	switch (state)
    {
		case RESULT_STATE_MAKING_SCREEN_SHOT:
            break;
			
		case RESULT_STATE_FINISHED:
			isFinished=true;
			break;
			
		default:
			break;
	}
}

void ResultScreen::Draw(const UIGeometricData &geometricData)
{
	Core *core=DAVA::Core::Instance();

	Vector2 screenSize(core->GetVirtualScreenWidth(), core->GetVirtualScreenHeight());
	
	float32 drawSize = Min(screenSize.x, screenSize.y);
	float32 scale = Min(drawSize / resultSprite->GetWidth(), drawSize / resultSprite->GetHeight());
	
    Sprite::DrawState state;
	state.SetPosition(0, 0);
	state.SetScale(scale, scale);
	
	resultSprite->Draw(&state);

    UIScreen::Draw(geometricData);
}

void ResultScreen::PrepareSprite()
{
	Rect r(0, 0, resultSprite->GetWidth(), resultSprite->GetHeight());

	RenderManager::Instance()->SetRenderTarget(resultSprite);
    
    Sprite::DrawState state;
    state.SetScale(RESULT_TEXTURE_SCALE, RESULT_TEXTURE_SCALE);
	textureSprite->Draw(&state);
	DrawStatImage(r);
	RenderManager::Instance()->RestoreRenderTarget();
}

void ResultScreen::DrawStatImage(Rect rect)
{
	RenderHelper *helper = RenderHelper::Instance();
	RenderManager *manager = RenderManager::Instance();

	for(uint32 i = 0; i < testData.GetItemCount(); ++i)
	{
		FpsStatItem item = testData.GetItem(i);
		Rect curRect = testData.TranslateRect(item.rect, rect);
		for(uint32 j = 0; j < SECTORS_COUNT; j++)
		{
			manager->SetColor(SettingsManager::Instance()->GetColorByFps(item.avFps[j]));
			Polygon2 curSector;
			curSector.AddPoint(curRect.GetCenter());
			curSector.AddPoint(GetVecInRect(curRect, DegToRad((SECTORS_COUNT - j) * 45.f - 22.5f)));
			curSector.AddPoint(GetVecInRect(curRect, DegToRad((SECTORS_COUNT - j) * 45.f)));
			curSector.AddPoint(GetVecInRect(curRect, DegToRad((SECTORS_COUNT - j) * 45.f + 22.5f)));
			helper->FillPolygon(curSector, RenderState::DEFAULT_2D_STATE);
			manager->SetColor(Color::Black);
			helper->DrawPolygon(curSector, true, RenderState::DEFAULT_2D_STATE);
		}
	}
}

Vector2 ResultScreen::GetVecInRect(const Rect & rect, float32 angleInRad)
{
	Vector2 retVec;
	Matrix2 m;
	m.BuildRotation(angleInRad);
	angleInRad += DAVA::PI_05;
	while(angleInRad > DAVA::PI_05)
		angleInRad -= DAVA::PI_05;
	if(angleInRad > DAVA::PI_05 / 2)
		angleInRad = DAVA::PI_05 - angleInRad;
	Vector2 v = Vector2((Point2f(rect.GetSize().x / 2, 0) * m).data) / Abs(cosf(angleInRad));
    
	retVec = v + rect.GetCenter();
	return retVec;
}
