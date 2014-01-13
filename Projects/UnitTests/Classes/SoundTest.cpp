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



#if 0

#include "SoundTest.h"

SoundTest::SoundTest():
TestTemplate<SoundTest>("SoundTest"),
sndClick(0),
effectPlayTest(false)
{
	RegisterFunction(this, &SoundTest::CreateInvalidSounds, "CreateInvalidSounds", NULL);
	RegisterFunction(this, &SoundTest::CreateValidSounds, "CreateValidSounds", NULL);

	RegisterFunction(this, &SoundTest::PlayStopEffect, "PlayStopEffect", NULL);
	RegisterFunction(this, &SoundTest::PlayStopMusic, "PlayStopMusic", NULL);
    
	RegisterFunction(this, &SoundTest::PlayEffect, "PlayEffect", NULL);
}

void SoundTest::LoadResources()
{
    GetBackground()->SetColor(Color::White);
    
    sndClick = Sound::CreateFX("~res:/Sounds/click.wav", Sound::TYPE_STATIC);
#ifdef __DAVAENGINE_IPHONE__
	music = Sound::CreateMusic("~res:/Sounds/map.caf", Sound::TYPE_STREAMED);
#else
	music = Sound::CreateMusic("~res:/Sounds/map.ogg", Sound::TYPE_STREAMED);
#endif
}

void SoundTest::UnloadResources()
{
    SafeRelease(sndClick);
    SafeRelease(music);
}

void SoundTest::CreateInvalidSounds(PerfFuncData * data)
{
    Sound * sound = 0;
    sound = Sound::Create(FilePath(), (Sound::eType)1111);
    TEST_VERIFY(sound == 0);
    sound = Sound::Create("?InvalidPath", Sound::TYPE_STATIC);
    TEST_VERIFY(sound == 0);
    
    sound = Sound::CreateFX(FilePath(), Sound::TYPE_STATIC);
    TEST_VERIFY(sound == 0);
    sound = Sound::CreateFX("//InvalidPath", (Sound::eType)333);
    TEST_VERIFY(sound == 0);
    
    sound = Sound::CreateMusic(FilePath(), (Sound::eType)222);
    TEST_VERIFY(sound == 0);
    sound = Sound::CreateMusic(":\\InvalidPath", Sound::TYPE_STREAMED);
    TEST_VERIFY(sound == 0);
    
#ifdef __DAVAENGINE_IPHONE__
	sound = Sound::CreateMusic("~res:/Sounds/null.caf", Sound::TYPE_STREAMED);
    TEST_VERIFY(sound == 0);
    SafeRelease(sound);
#else
	sound = Sound::CreateMusic("~res:/Sounds/null.ogg", Sound::TYPE_STREAMED);
    TEST_VERIFY(sound == 0);
    SafeRelease(sound);
#endif
}

void SoundTest::CreateValidSounds(PerfFuncData * data)
{
    Sound * sound = 0;
    
    sound = Sound::CreateFX("~res:/Sounds/click.wav", Sound::TYPE_STATIC);
    TEST_VERIFY(sound != 0);
    SafeRelease(sound);
    
#ifdef __DAVAENGINE_IPHONE__
	sound = Sound::CreateMusic("~res:/Sounds/map.caf", Sound::TYPE_STREAMED);
    TEST_VERIFY(sound != 0);
    SafeRelease(sound);
#else
	sound = Sound::CreateMusic("~res:/Sounds/map.ogg", Sound::TYPE_STREAMED);
    TEST_VERIFY(sound != 0);
    SafeRelease(sound);
#endif
    
}

void SoundTest::PlayStopEffect(PerfFuncData * data)
{
    SoundInstance * clickInst = sndClick->Play();
    TEST_VERIFY(clickInst->GetState() == SoundInstance::STATE_PLAYING);
    
    sndClick->Stop();
    TEST_VERIFY(clickInst->GetState() == SoundInstance::STATE_FORCED_STOPPED);
}

void SoundTest::PlayStopMusic(PerfFuncData * data)
{
    SoundInstance * musicInst = music->Play();
    TEST_VERIFY(musicInst->GetState() == SoundInstance::STATE_PLAYING);
    
    music->Stop();
    TEST_VERIFY(musicInst->GetState() == SoundInstance::STATE_FORCED_STOPPED);
}

void SoundTest::PlayEffect(PerfFuncData * data)
{
    effectIns = sndClick->Play();
    effectPlayTest = true;
}

void SoundTest::Update(float32 timeElapsed)
{
    if(effectPlayTest)
    {
        if(effectIns->GetState() == SoundInstance::STATE_PLAYING)
            return;
        else
        {
            PerfFuncData * data = new PerfFuncData();
            TEST_VERIFY(effectIns->GetState() == SoundInstance::STATE_COMPLETED);
            SafeDelete(data);
        }
    }

    TestTemplate<SoundTest>::Update(timeElapsed);
}

#endif