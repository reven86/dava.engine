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


#include "UIParticlesTest.h"

static const float LIST_TEST_AUTO_CLOSE_TIME = 30.0f;

UIParticlesTest::UIParticlesTest() :
 TestTemplate<UIParticlesTest>("UIParticlesTest")
{
	testFinished = false;
	
	RegisterFunction(this, &UIParticlesTest::TestFunction, Format("UIParticlesTest"), NULL);
	onScreenTime = 0.f;
}

void UIParticlesTest::LoadResources()
{
	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(14);

	finishTestBtn = new UIButton(Rect(10, 510, 300, 30));
	finishTestBtn->SetStateFont(0xFF, font);
	finishTestBtn->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
	finishTestBtn->SetStateText(0xFF, L"Finish test");

	finishTestBtn->SetDebugDraw(true);
	finishTestBtn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIParticlesTest::ButtonPressed));
	AddControl(finishTestBtn);
	
	UIButton *loadFromYamlButton = new UIButton(Rect(650, 520, 300, 30));
	loadFromYamlButton->SetName("LoadYaml");
	loadFromYamlButton->SetStateFont(0xFF, font);
	loadFromYamlButton->SetStateText(0xFF, L"Load from yaml");
	loadFromYamlButton->SetDebugDraw(true);
	loadFromYamlButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIParticlesTest::ButtonPressed));
	AddControl(loadFromYamlButton);
	SafeRelease(loadFromYamlButton);

	UIButton *startButton = new UIButton(Rect(650, 550, 300, 30));
	startButton->SetName("StartParticles");
	startButton->SetStateFont(0xFF, font);
	startButton->SetStateText(0xFF, L"Start");
	startButton->SetDebugDraw(true);
	startButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIParticlesTest::ButtonPressed));
	AddControl(startButton);
	SafeRelease(startButton);
	
	UIButton *pauseButton = new UIButton(Rect(650, 580, 300, 30));
	pauseButton->SetName("PauseParticles");
	pauseButton->SetStateFont(0xFF, font);
	pauseButton->SetStateText(0xFF, L"Pause");
	pauseButton->SetDebugDraw(true);
	pauseButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIParticlesTest::ButtonPressed));
	AddControl(pauseButton);
	SafeRelease(pauseButton);
	
	UIButton *stopButton = new UIButton(Rect(650, 610, 300, 30));
	stopButton->SetName("StopParticles");
	stopButton->SetStateFont(0xFF, font);
	stopButton->SetStateText(0xFF, L"Stop");
	stopButton->SetDebugDraw(true);
	stopButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIParticlesTest::ButtonPressed));
	AddControl(stopButton);
	SafeRelease(stopButton);

	SafeRelease(font);
}

void UIParticlesTest::UnloadResources()
{
	RemoveAllControls();
	SafeRelease(finishTestBtn);
}

void UIParticlesTest::DidAppear()
{
    onScreenTime = 0.f;
}

void UIParticlesTest::Update(float32 timeElapsed)
{
    onScreenTime += timeElapsed;
    if(onScreenTime > LIST_TEST_AUTO_CLOSE_TIME)
    {
        testFinished = true;
    }
    
    TestTemplate<UIParticlesTest>::Update(timeElapsed);
}

void UIParticlesTest::TestFunction(PerfFuncData * data)
{
	return;
}

bool UIParticlesTest::RunTest(int32 testNum)
{
	TestTemplate<UIParticlesTest>::RunTest(testNum);
	return testFinished;
}

void UIParticlesTest::StartParticles()
{
	const List<UIControl*> &allchildren = GetChildren();
	List<UIControl*>::const_iterator it;
		
	for(it = allchildren.begin(); it != allchildren.end(); it++)
	{
		UIControl *control = (*it);
		UIParticles *particle = dynamic_cast<UIParticles *>(control);
		if (particle)
		{
			if (!particle->IsPaused() && particle->IsStopped())
			{
				particle->Start();
			}
		}
	}
}

void UIParticlesTest::StopParticles()
{
	const List<UIControl*> &allchildren = GetChildren();
	List<UIControl*>::const_iterator it;
		
	for(it = allchildren.begin(); it != allchildren.end(); it++)
	{
		UIControl *control = (*it);
		UIParticles *particle = dynamic_cast<UIParticles *>(control);
		if (particle)
		{
			particle->Stop();
		}
	}
}

void UIParticlesTest::PauseParticles()
{
	const List<UIControl*> &allchildren = GetChildren();
	List<UIControl*>::const_iterator it;
		
	for(it = allchildren.begin(); it != allchildren.end(); it++)
	{
		UIControl *control = (*it);
		UIParticles *particle = dynamic_cast<UIParticles *>(control);
		if (particle)
		{
			particle->Pause(!particle->IsPaused());
		}
	}
}

void UIParticlesTest::LoadParticlesFromYaml()
{
	UIYamlLoader::Load( this, "~res:/UI/Test/Particles.yaml" );
	particle1 = DynamicTypeCheck<UIParticles *>(FindByName("UIParticles1"));
	particle2 = DynamicTypeCheck<UIParticles *>(FindByName("UIParticles2"));
	
    UIYamlLoader::Load( this, "~res:/UI/Test/Particles2.yaml" );
	particle3 = DynamicTypeCheck<UIParticles *>(FindByName("UIParticles3"));
	particle4 = DynamicTypeCheck<UIParticles *>(FindByName("UIParticles4"));
}


void UIParticlesTest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
	UIControl *control = dynamic_cast<UIControl*>(obj);
	if (control)
	{
		if (control->GetName() == "LoadYaml")
		{
			LoadParticlesFromYaml();
		}
		else if (control->GetName() == "PauseParticles")
		{
			PauseParticles();
		}
		else if (control->GetName() == "StopParticles")
		{
			StopParticles();
		}
		else if (control->GetName() == "StartParticles")
		{
			StartParticles();
		}
	}

	if (obj == finishTestBtn)
	{
		testFinished = true;
	}
}
