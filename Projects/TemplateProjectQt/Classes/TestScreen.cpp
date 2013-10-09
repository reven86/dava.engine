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


#include "TestScreen.h"

void TestScreen::LoadResources()
{
	manager = GameObjectManager::Create();
	bigBox = GameObject::Create(FilePath("~res:/Gfx/GameObjects/blueboxbig"));
	bigBox->SetPivotPoint(ALIGN_HCENTER | ALIGN_VCENTER);
	bigBox->SetPosition(200, 200);
	manager->AddObject(bigBox.Get());

	smallBox = GameObject::Create(FilePath("~res:/Gfx/GameObjects/bluebox"));
	smallBox->SetPivotPoint(ALIGN_HCENTER | ALIGN_VCENTER);
	bigBox->AddObject(smallBox.Get());

	smallCircle = GameObject::Create(FilePath("~res:/Gfx/GameObjects/bluecircle"));
	smallCircle->SetPosition(bigBox->GetSize());
	smallCircle->SetPivotPoint(ALIGN_HCENTER | ALIGN_VCENTER);
	bigBox->AddObject(smallCircle.Get());
	
	bigBox->SetAngle(DegToRad(30.0f));
	time = 0.0f;
	
	bigBoxParticles = GameObject::Create(FilePath("~res:/Gfx/GameObjects/blueboxbig"));
	bigBoxParticles->SetPivotPoint(ALIGN_HCENTER | ALIGN_VCENTER);
	bigBoxParticles->SetPosition(200, 100);
	manager->AddObject(bigBoxParticles.Get());
	
	bigBoxEmitter = new ParticleEmitterObject(); 
	bigBoxEmitter->LoadFromYaml(FilePath("~res:/Particles/sparkles.yaml"));
	bigBoxEmitter->SetPriority(10);
	
//	RefPtr<GameObject> centerTest  = GameObject::Create("~res:/Gfx/GameObjects/bluecircle");
//	centerTest->SetPosition(0, 0);
//	centerTest->SetPivotPoint(ALIGN_HCENTER | ALIGN_VCENTER);
//	bigBoxEmitter->AddObject(centerTest.Get());

	smallCircle->AddObject(bigBoxEmitter.Get());
	
	ParticleEmitterObject* bigBoxEmitterClone = new ParticleEmitterObject();
	bigBoxEmitterClone->SetEmitter(bigBoxEmitter->GetEmitter()->Clone());
	bigBoxEmitterClone->SetPriority(10);
	bigBoxEmitterClone->SetPosition(Vector2(300, 300));
	manager->AddObject(bigBoxEmitterClone);

	Texture::EnableMipmapGeneration();
	testSprite = Sprite::Create(FilePath("~res:/Gfx/GameObjects/blueboxbig"));
	Texture::DisableMipmapGeneration();

	sndClick = Sound::CreateFX(FilePath("~res:/Sounds/click.wav"), Sound::TYPE_STATIC);

	music = Sound::CreateMusic(FilePath("~res:/Sounds/final.ogg"), Sound::TYPE_STREAMED);
	music->SetLooping(true);

	isPlaying = false;

	UIControl *btn = new UIControl(Rect(0, 0, 100, 30));
	btn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &TestScreen::OnButton));
	btn->SetInputEnabled(true);

	btn->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
	btn->GetBackground()->SetColor(Color::White());

	AddControl(btn);
	SafeRelease(btn);

	backColor = Color(1.0f, 0.0f, 0.0f, 1.0f);
}

void TestScreen::UnloadResources()
{
	SafeRelease(testSprite);
	SafeRelease(manager);
	SafeRelease(sndClick);
	SafeRelease(music);
}

void TestScreen::WillAppear()
{
	music->Play();
	isPlaying = true;
}

void TestScreen::WillDisappear()
{
	
}

void TestScreen::Input(UIEvent * event)
{
	if (event->phase == UIEvent::PHASE_KEYCHAR)
	{
		if (event->keyChar == '1')
		{
			Core::Instance()->ToggleFullscreen();
		}
	}
	if (event->phase == UIEvent::PHASE_BEGAN)
	{
		if(isPlaying)
		{
			music->Stop();
			isPlaying = false;
		}
		else
		{
			music->Play();
			isPlaying = true;
		}
		sndClick->Play();
	}
}

void TestScreen::Update(float32 timeElapsed)
{
	// scaling test
	time += timeElapsed;
	float32 scale = 1.0f + 0.3f * sinf(time);
	bigBox->SetScale(scale, scale);
					
	// 
	bigBox->SetAngle(bigBox->GetAngle() + DegToRad(5.0f) * timeElapsed);
	smallBox->SetAngle(smallBox->GetAngle() + DegToRad(20.0f) * timeElapsed);
	smallCircle->SetAngle(smallCircle->GetAngle() - DegToRad(20.0f) * timeElapsed);
	manager->Update(timeElapsed);
}

void TestScreen::Draw(const UIGeometricData &geometricData)
{
    RenderManager::Instance()->SetColor(backColor);
    RenderHelper::Instance()->FillRect(GetRect());
    
    manager->Draw();

	RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
//	RenderManager::Instance()->SetColor(backColor);

	testSprite->SetPosition(0.0f, 200.0f);
	testSprite->Draw();

	testSprite->SetPosition(200.0f, 200.0f);
	testSprite->SetScale(0.5f, 0.5f);
	testSprite->Draw();

	testSprite->SetPosition(300.0f, 200.0f);
	testSprite->SetScale(0.3f, 0.3f);
	testSprite->Draw();

	testSprite->SetPosition(400.0f, 200.0f);
	testSprite->SetScale(0.1f, 0.1f);
	testSprite->Draw();

	UIScreen::Draw(geometricData);
}


void TestScreen::OnButton(BaseObject *, void *, void *)
{
	backColor.r += 0.7f;
	backColor.g += 0.3f;

	if(1.0f < backColor.r)
	{
		backColor.r = 0.0f;
	}

	if(1.0f < backColor.g)
	{
		backColor.g = 0.02f;
	}
}