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



#include "FileSystem/FilePath.h"
#include "Sound/SoundSystem.h"

#ifdef __DAVAENGINE_IPHONE__

#include "musicios.h"
#import <AVFoundation/AVFoundation.h>

@interface AvSound : NSObject <AVAudioPlayerDelegate>
{
@public
	AVAudioPlayer		*audioPlayer;

    DAVA::MusicIOSSoundEvent * musicEvent;
	
	BOOL				playing;
	BOOL				interruptedOnPlayback;
    BOOL                initSuccess;
}

@property(nonatomic, readonly) AVAudioPlayer * audioPlayer;

- (bool)play;
- (void)stop;
- (void)pause;
 
@end

@implementation AvSound

@synthesize audioPlayer;

- (id)initWithFileName: (NSString*)name withMusicEvent: (DAVA::MusicIOSSoundEvent*)event
{  
    initSuccess = true;
    musicEvent = event;
    if(self == [super init])
	{  
        NSError * error = 0;
        NSURL * url = [NSURL fileURLWithPath: name];
		audioPlayer	= [[AVAudioPlayer alloc] initWithContentsOfURL: url error: &error];
		if(error)
        {
            NSLog(@"AvSound::initWithFileName error %s", [[error localizedDescription] cStringUsingEncoding:NSASCIIStringEncoding]);
            initSuccess = false;
        }
        else
        {
            audioPlayer.delegate = self;
            [audioPlayer prepareToPlay];
        }
	}
	
    return self;  
}  

- (void)dealloc  
{  
	[audioPlayer release];

    [super dealloc];  
} 

- (bool)play
{
	playing = YES;
	return [audioPlayer play];
}

- (void)stop
{
	playing = NO;
	[audioPlayer stop];
    audioPlayer.currentTime = 0;
}

- (void)pause
{
	playing = NO;
	[audioPlayer pause];
}

- (void)audioPlayerBeginInterruption: (AVAudioPlayer*)player
{
    if (playing)
	{
        playing					= NO;
        interruptedOnPlayback	= YES;
    }
}

- (void)audioPlayerEndInterruption:(AVAudioPlayer *)player withOptions:(NSUInteger)flags
{
    if (interruptedOnPlayback)
	{
        [player prepareToPlay];
        [player play];

        playing					= YES;
        interruptedOnPlayback	= NO; 
    }
}

- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag
{
    musicEvent->PerformEndCallback();
}

@end



namespace DAVA
{

MusicIOSSoundEvent * MusicIOSSoundEvent::CreateMusicEvent(const FilePath & path)
{
    MusicIOSSoundEvent * event = new MusicIOSSoundEvent(path);
    if(event->Init())
        return event;
    
    Logger::Error("[MusicIOSSoundEvent::CreateMusicEvent] failed to create music event: %s",
                           path.GetAbsolutePathname().c_str());
    
    SafeRelease(event);
    return 0;
}
    
MusicIOSSoundEvent::MusicIOSSoundEvent(const FilePath & path) :
    avSound(0),
    filePath(path)
{

}

bool MusicIOSSoundEvent::Init()
{
    avSound = [[AvSound alloc] initWithFileName:
               [NSString stringWithCString:filePath.GetAbsolutePathname().c_str() encoding:NSASCIIStringEncoding]
               withMusicEvent:this];
    
    if(avSound && !((AvSound *)avSound)->initSuccess)
    {
        [(AvSound*)avSound release];
        avSound = 0;
        return false;
    }
    return true;
}
    
MusicIOSSoundEvent::~MusicIOSSoundEvent()
{
    [(AvSound*)avSound release];
    avSound = 0;
    
    SoundSystem::Instance()->RemoveSoundEventFromGroups(this);
}

bool MusicIOSSoundEvent::Trigger()
{
    Retain();
    return [(AvSound*)avSound play];
}

void MusicIOSSoundEvent::Stop(bool force /* = false */)
{
    [(AvSound*)avSound stop];
}

void MusicIOSSoundEvent::SetPaused(bool paused)
{
    if(paused)
        [(AvSound*)avSound pause];
    else
        [(AvSound*)avSound play];
}
    
void MusicIOSSoundEvent::SetVolume(float32 _volume)
{
	volume = _volume;
	((AvSound*)avSound).audioPlayer.volume = Clamp(_volume, 0.f, 1.f);
}
    
void MusicIOSSoundEvent::SetLoopCount(int32 looping)
{
    ((AvSound*)avSound).audioPlayer.numberOfLoops = looping;
}
    
int32 MusicIOSSoundEvent::GetLoopCount() const
{
    return static_cast<int32>(((AvSound*)avSound).audioPlayer.numberOfLoops);
}

bool MusicIOSSoundEvent::IsActive() const
{
    return [((AvSound*)avSound).audioPlayer isPlaying];
}

void MusicIOSSoundEvent::PerformEndCallback()
{
    SoundSystem::Instance()->ReleaseOnUpdate(this);
    PerformEvent(DAVA::MusicIOSSoundEvent::EVENT_END);
}
    
};
#endif //#ifdef __DAVAENGINE_IPHONE__