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

#include "SoundTest.h"

SoundTest::SoundTest():
TestTemplate<SoundTest>("SoundTest"),
sndClick(0),
music(0),
effectPlayTest(true)
{
	RegisterFunction(this, &SoundTest::CreateInvalidSounds, "CreateInvalidSounds", NULL);
	RegisterFunction(this, &SoundTest::CreateValidSounds, "CreateValidSounds", NULL);
    
	RegisterFunction(this, &SoundTest::PlayStopEffect, "PlayStopEffect", NULL);
	RegisterFunction(this, &SoundTest::PlayStopMusic, "PlayStopMusic", NULL);
    
	RegisterFunction(this, &SoundTest::PlayEffects, "PlayEffect", NULL);
}

void SoundTest::LoadResources()
{
    GetBackground()->SetColor(Color::White);
	
	sndClick = SoundSystem::Instance()->CreateSoundEventFromFile("~res:/Sounds/click.wav", FastName("clickEvent"));
    
#ifdef __DAVAENGINE_IPHONE__
	music = SoundSystem::Instance()->CreateSoundEventFromFile("~res:/Sounds/map.caf", FastName("musicEvent"));
#else
	music = SoundSystem::Instance()->CreateSoundEventFromFile("~res:/Sounds/map.ogg", FastName("musicEvent"));
#endif
}

void SoundTest::UnloadResources()
{
    SafeRelease(sndClick);
    SafeRelease(music);
}

void SoundTest::CreateInvalidSounds(PerfFuncData * data)
{
    SoundEvent * soundEvent = 0;
    soundEvent = SoundSystem::Instance()->CreateSoundEventFromFile(FilePath(), FastName("1111"));
    TEST_VERIFY(soundEvent == 0);
    soundEvent = SoundSystem::Instance()->CreateSoundEventFromFile("?InvalidPath", FastName("soundEvent"));
    TEST_VERIFY(soundEvent == 0);
    
    soundEvent = SoundSystem::Instance()->CreateSoundEventFromFile(FilePath(), FastName("soundEvent"));
    TEST_VERIFY(soundEvent == 0);
    soundEvent = SoundSystem::Instance()->CreateSoundEventFromFile("//InvalidPath", FastName("333"));
    TEST_VERIFY(soundEvent == 0);
    
    soundEvent = SoundSystem::Instance()->CreateSoundEventFromFile(FilePath(), FastName("222"));
    TEST_VERIFY(soundEvent == 0);
    soundEvent = SoundSystem::Instance()->CreateSoundEventFromFile(":\\InvalidPath", FastName("soundEvent"));
    TEST_VERIFY(soundEvent == 0);
    
#ifdef __DAVAENGINE_IPHONE__
	soundEvent = SoundSystem::Instance()->CreateSoundEventFromFile("~res:/Sounds/null.caf", FastName("soundEvent"));
    TEST_VERIFY(soundEvent == 0);
    SafeRelease(soundEvent);
#else
	soundEvent = SoundSystem::Instance()->CreateSoundEventFromFile("~res:/Sounds/null.ogg", FastName("soundEvent"));
    TEST_VERIFY(soundEvent == 0);
    SafeRelease(soundEvent);
#endif
}

void SoundTest::CreateValidSounds(PerfFuncData * data)
{
	SoundEvent *soundEvent = 0;
    
	soundEvent = SoundSystem::Instance()->CreateSoundEventFromFile("~res:/Sounds/click.wav", FastName("clickEvent"));
    TEST_VERIFY(soundEvent != 0);
    SafeRelease(soundEvent);
    
#ifdef __DAVAENGINE_IPHONE__
	soundEvent = SoundSystem::Instance()->CreateSoundEventFromFile("~res:/Sounds/map.caf", FastName("musicEvent"));
    TEST_VERIFY(soundEvent != 0);
    SafeRelease(soundEvent);
#else
	soundEvent = SoundSystem::Instance()->CreateSoundEventFromFile("~res:/Sounds/map.ogg", FastName("musicEvent"));
    TEST_VERIFY(soundEvent != 0);
    SafeRelease(soundEvent);
#endif
    
}

void SoundTest::PlayStopEffect(PerfFuncData * data)
{
	TEST_VERIFY(sndClick->Trigger());
	TEST_VERIFY(sndClick->IsActive());
	sndClick->Stop();
	TEST_VERIFY(!sndClick->IsActive());
}

void SoundTest::PlayStopMusic(PerfFuncData * data)
{
	TEST_VERIFY(music->Trigger());
	TEST_VERIFY(music->IsActive());
	music->Stop();
	TEST_VERIFY(!music->IsActive());
}

void SoundTest::PlayEffects(PerfFuncData * data)
{
	TEST_VERIFY(sndClick != 0);
	sndClick->AddEvent(SoundEvent::EVENT_END, Message(this, &SoundTest::SoundEventPlayed));
	effectPlayTest = false;
	sndClick->Trigger();
    
    //	TEST_VERIFY(music != 0)
    //	music->AddEvent(SoundEvent::EVENT_END, Message(this, &SoundTest::SoundEventPlayed));
    //	music->Trigger();
}

void SoundTest::SoundEventPlayed(BaseObject *obj, void *data, void *callerData)
{
	effectPlayTest = true;
}

void SoundTest::Update(float32 timeElapsed)
{
	if (!effectPlayTest)
	{
		return;
	}
    
    TestTemplate<SoundTest>::Update(timeElapsed);
}
