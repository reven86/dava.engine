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
#include "TestChooserScreen.h"


TestChooserScreen::TestChooserScreen(const Vector<BaseTest*>& _testsChain) : 
	testForRun(nullptr)
{
	testChain = _testsChain;
}


TestChooserScreen::~TestChooserScreen()
{
}

void TestChooserScreen::OnStart(HashMap<String, BaseObject*>& params)
{
	CreateChooserUI();
}

void TestChooserScreen::OnFinish(HashMap<String, BaseObject*>& params)
{

}

bool TestChooserScreen::IsFinished() const
{
	return testForRun != nullptr;
}

void TestChooserScreen::BeginFrame()
{
	RenderSystem2D::Instance()->Reset();
	RenderManager::Instance()->BeginFrame();
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
}

void TestChooserScreen::EndFrame()
{
	RenderManager::Instance()->EndFrame();
	RenderManager::Instance()->ProcessStats();
}

void TestChooserScreen::Update(float32 timeElapsed)
{
	UIControlSystem::Instance()->Update();
}

void TestChooserScreen::Draw()
{
	UIControlSystem::Instance()->Draw();
}

void TestChooserScreen::CreateChooserUI()
{
	UIScreen* chooserScreen = new UIScreen();

	UIScreenManager::Instance()->RegisterScreen(0, chooserScreen);
	UIScreenManager::Instance()->SetFirst(0);

	Font* font = FTFont::Create("./Data/Fonts/korinna.ttf");
	uint32 offsetY = 150;
	uint32 testNumber = 0;

	for each (BaseTest* test in testChain)
	{
		if (test->GetDebugFrame() > 0)
		{
			UIButton* button = new UIButton();

			button->SetPosition(Vector2(10.0f, 10.0f));
			button->SetStateFont(UIButton::DRAW_STATE_UNPRESSED, font);
			button->SetStateText(UIButton::DRAW_STATE_UNPRESSED, UTF8Utils::EncodeToWideString(test->GetName()));
			button->SetStateTextAlign(UIButton::DRAW_STATE_UNPRESSED, ALIGN_LEFT | ALIGN_VCENTER);
			button->SetSize(Vector2(150.0f, 10.0f));
		}
	}
}