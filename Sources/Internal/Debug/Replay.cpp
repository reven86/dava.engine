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


#include "Replay.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileList.h"
#include "Utils/Random.h"
#include "Debug/DVAssert.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"

namespace DAVA
{

bool Replay::isRecord = false;
bool Replay::isPlayback = false;

Replay::Replay()
:	file(0)
{

}

Replay::~Replay()
{
	StopRecord();
}

void Replay::StartRecord(const FilePath & dirName)
{	
	DVASSERT(!isRecord);
	DVASSERT(!isPlayback);
	isRecord = true;
	pauseReplay = false;

	FileSystem::Instance()->DeleteDirectoryFiles(dirName, false);
	FileSystem::Instance()->CreateDirectory(dirName);
    
    FileList * list = new FileList("~doc:/");
    int32 listSize = list->GetCount();
    for(int32 i = 0; i < listSize; ++i)
    {
        String fileName = list->GetFilename(i);
        if(!list->IsNavigationDirectory(i) && !list->IsDirectory(i) && fileName != "LastReplay.rep")
        {
            FileSystem::Instance()->CopyFile(list->GetPathname(i), dirName + fileName);
        }
    }

    list->Release();

    FilePath filePath = dirName + "LastReplay.rep";
    file = File::Create(filePath, File::CREATE | File::WRITE);

    Random::Instance()->Seed();
}

void Replay::StopRecord()
{
	isRecord = false;
	SafeRelease(file);
}

void Replay::RecordFrame(float32 frameTime)
{
	//Logger::FrameworkDebug("[Replay::RecordFrame] %f", frameTime);
	Write<int8>(VALUE_FRAMETIME);
	Write(frameTime);
}

void Replay::RecordEventsCount(int32 eventsCount)
{
	//Logger::FrameworkDebug("[Replay::RecordEventsCount] %d", eventsCount);
	Write<int8>(VALUE_EVENTS_COUNT);
	Write(eventsCount);
}

void Replay::RecordEvent(const UIEvent * ev)
{
	Write(ev->tid);
	Write(ev->point.x);
	Write(ev->point.y);
	Write(ev->timestamp);
	Write(ev->phase);
	Write(ev->controlState);
	Write(ev->tapCount);
	Write<uint16>(ev->keyChar);
    Write(static_cast<uint32>(ev->device));
}

void Replay::RecordSeed(const uint32 seed)
{
	Write<int8>(VALUE_SEED);
	Write(seed);
}

void Replay::StartPlayback(const FilePath & dirName)
{
	DVASSERT(!isRecord);
	DVASSERT(!isPlayback);
	pauseReplay = false;
	isPlayback = true;

	FileSystem::Instance()->DeleteDirectoryFiles("~doc:/", false);
	FileList * list = new FileList(dirName);
	int32 listSize = list->GetCount();
	for(int32 i = 0; i < listSize; ++i)
	{
		String fileName = list->GetFilename(i);
		if(!list->IsNavigationDirectory(i) && !list->IsDirectory(i))
		{
            FilePath existingFile = dirName + fileName;
            FilePath newFile("~doc:/" + fileName);
            
			FileSystem::Instance()->CopyFile(existingFile, newFile);
		}
	}

	list->Release();


	skipType = false;
	file = File::Create("~doc:/LastReplay.rep", File::OPEN | File::READ);
}

float32 Replay::PlayFrameTime()
{
	if(!skipType)
	{
		int8 type = Read<int8>();
		if(!isPlayback) return 0.f;
		while(VALUE_SEED == type)
		{
			Random::Instance()->Seed(Read<uint32>());
			if(!isPlayback) return 0.f;
			type = Read<int8>();
			if(!isPlayback) return 0.f;
		}
	}
	else
	{
		skipType = false;
	}
	float32 ret = Read<float32>();
	if(!isPlayback) return 0.f;
	return ret;
}

int32 Replay::PlayEventsCount()
{
	if(!skipType)
	{
		Read<int8>();
		if(!isPlayback) return 0;
	}
	else
	{
		skipType = false;
	}

	int32 ret = Read<int32>();
	if(!isPlayback) return 0;
	return ret;
}

UIEvent	Replay::PlayEvent()
{
	UIEvent ev;

	ev.tid = Read<int32>(); if(!isPlayback) return ev;
	ev.point.x = Read<float32>(); if(!isPlayback) return ev;
	ev.point.y = Read<float32>(); if(!isPlayback) return ev;
	ev.timestamp = Read<float64>(); if(!isPlayback) return ev;
    ev.phase = static_cast<UIEvent::Phase>(Read<int32>());
    if (!isPlayback)
        return ev;
	ev.controlState = Read<int32>(); if(!isPlayback) return ev;
	ev.tapCount = Read<int32>(); if(!isPlayback) return ev;
    ev.keyChar = Read<uint16>();
    if (!isPlayback)
        return ev;
    ev.device = Read<UIEvent::Device>();
    if (!isPlayback)
        return ev;

    return ev;
}

void Replay::PlaySeed()
{

}

bool Replay::IsEvent()
{
	int8 type = Read<int8>();
	while(VALUE_SEED == type)
	{
		Random::Instance()->Seed(Read<uint32>());
		type = Read<int8>();
	}

	skipType = true;
	return type == VALUE_EVENTS_COUNT;
}

void Replay::PauseReplay(bool isPause)
{
	pauseReplay = isPause;
}

};
