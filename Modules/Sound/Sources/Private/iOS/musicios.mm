#include "Logger/Logger.h"
#include "FileSystem/FilePath.h"
#include "FMODSoundSystem.h"

#include "musicios.h"
#import <AVFoundation/AVFoundation.h>

@interface AvSound : NSObject<AVAudioPlayerDelegate>
{
@public
    AVAudioPlayer* audioPlayer;

    DAVA::MusicIOSSoundEvent* musicEvent;

    BOOL playing;
    BOOL interruptedOnPlayback;
    BOOL initSuccess;
}

@property(nonatomic, readonly) AVAudioPlayer* audioPlayer;

- (bool)play;
- (void)stop;
- (void)pause;

@end

@implementation AvSound

@synthesize audioPlayer;

- (id)initWithFileName:(NSString*)name withMusicEvent:(DAVA::MusicIOSSoundEvent*)event
{
    initSuccess = true;
    musicEvent = event;
    if (self == [super init])
    {
        NSError* error = 0;
        NSURL* url = [NSURL fileURLWithPath:name];
        audioPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error];
        if (error)
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

- (void)audioPlayerBeginInterruption:(AVAudioPlayer*)player
{
    if (playing)
    {
        playing = NO;
        interruptedOnPlayback = YES;
    }
}

- (void)audioPlayerEndInterruption:(AVAudioPlayer*)player withOptions:(NSUInteger)flags
{
    if (interruptedOnPlayback)
    {
        [player prepareToPlay];
        [player play];

        playing = YES;
        interruptedOnPlayback = NO;
    }
}

- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer*)player successfully:(BOOL)flag
{
    musicEvent->PerformEndCallback();
}

@end

namespace DAVA
{
MusicIOSSoundEvent* MusicIOSSoundEvent::CreateMusicEvent(const FilePath& path, FMODSoundSystem* fmodSoundSystem)
{
    MusicIOSSoundEvent* event = new MusicIOSSoundEvent(path, fmodSoundSystem);
    if (event->Init())
        return event;

    Logger::Error("[MusicIOSSoundEvent::CreateMusicEvent] failed to create music event: %s",
                  path.GetAbsolutePathname().c_str());

    SafeRelease(event);
    return 0;
}

MusicIOSSoundEvent::MusicIOSSoundEvent(const FilePath& path, FMODSoundSystem* fmodSoundSystem)
    : avSound(0)
    , soundSystem(fmodSoundSystem)
    , filePath(path)
{
}

bool MusicIOSSoundEvent::Init()
{
    avSound = [[AvSound alloc] initWithFileName:
                               [NSString stringWithCString:filePath.GetAbsolutePathname().c_str()
                                                  encoding:NSASCIIStringEncoding]
                                 withMusicEvent:this];

    if (avSound && !((AvSound*)avSound)->initSuccess)
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

    soundSystem->RemoveSoundEventFromGroups(this);
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
    if (paused)
        [(AvSound*)avSound pause];
    else
        [(AvSound*)avSound play];
}

void MusicIOSSoundEvent::SetVolume(float32 _volume)
{
    volume = _volume;
    ((AvSound*)avSound).audioPlayer.volume = Clamp(_volume, 0.f, 1.f);
}

void MusicIOSSoundEvent::SetSpeed(float32 _speed)
{
    speed = _speed;

    AVAudioPlayer* player = ((AvSound*)avSound).audioPlayer;
    if (!FLOAT_EQUAL(_speed, 1.0f))
    {
        // AVAudioPlayer only supports 0.5f - 2.0f range
        player.enableRate = YES;
        player.rate = Clamp(_speed, 0.5f, 2.0f);
    }
    else
    {
        player.enableRate = NO;
    }

    [player prepareToPlay];
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
    soundSystem->ReleaseOnUpdate(this);
    PerformEvent(DAVA::MusicIOSSoundEvent::EVENT_END);
}
};
