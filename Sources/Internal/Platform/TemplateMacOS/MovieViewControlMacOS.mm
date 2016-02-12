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


#include "Platform/TemplateMacOS/MovieViewControlMacOS.h"
#include "Platform/TemplateMacOS/CorePlatformMacOS.h"
#include "FileSystem/Logger.h"

#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <AppKit/AppKit.h>

enum MoviePlayerHelperState
{
    eStateNone,
    eStateInitializing,
    eStateInitializedOK,
    eStateInitializedWithError
};

enum MoviePlayerHelperPlaybackState
{
    eNone,
    eStopped,
    ePlayback,
    ePaused,
};

// MacOS movie player helper which uses AVFoundation logic to play video.
@interface MoviePlayerHelper : NSObject
{
    AVPlayer* videoPlayer;
    NSView* videoView;

    // Current player helper state.
    MoviePlayerHelperState playerState;

    // Playback state.
    MoviePlayerHelperPlaybackState playbackState;

    // Cached video parameters, which are to be applied after initialization.
    DAVA::Rect videoRect;

    // Whether the video screen is visible.
    bool videoVisible;

    // Scaling mode for newly open media
    DAVA::eMovieScalingMode scalingMode;
    double videoDuration;
}

// Set the video rectangle.
- (void)setRect:(const DAVA::Rect&)rect;

// Set the visible flag.
- (void)setVisible:(bool)isVisible;

// Load the movie in async way.
- (void)loadMovie:(NSURL*)movieURL scalingMode:(DAVA::eMovieScalingMode)desiredScalingMode;

// Playback control.
- (void)play;
- (void)pause;
- (void)stop;

// Whether the movie is playing?
- (bool)isPlaying;

@end

@implementation MoviePlayerHelper

- (MoviePlayerHelper*)init
{
    if (self = [super init])
    {
        videoPlayer = nil;
        videoView = nil;

        playbackState = eNone;
        playerState = eStateNone;
        videoVisible = true;

        scalingMode = DAVA::scalingModeNone;
        videoDuration = 0.0;
    }
    return self;
}

- (void)dealloc
{
    [videoView removeFromSuperview];
    [videoView release];
    videoView = nil;

    [videoPlayer release];
    videoPlayer = nil;

    [super dealloc];
}

- (void)loadMovie:(NSURL*)movieURL scalingMode:(DAVA::eMovieScalingMode)desiredScalingMode
{
    if (videoPlayer != nullptr)
    {
        [videoView removeFromSuperview];
        [videoView release];
        videoView = nil;

        [videoPlayer release];
        videoPlayer = nil;
    }

    videoPlayer = [[AVPlayer alloc] init];

    scalingMode = desiredScalingMode;
    playerState = eStateInitializing;

    AVAsset* asset = [AVAsset assetWithURL:movieURL];
    NSArray* assetKeysToLoadAndTest = [NSArray arrayWithObjects:@"playable", @"tracks", @"duration", nil];
    [asset loadValuesAsynchronouslyForKeys:assetKeysToLoadAndTest
                         completionHandler:^(void) {
                           // The asset invokes its completion handler on an arbitrary queue when loading is complete.
                           // Because we want to access our AVPlayer in our ensuing set-up, we must dispatch our handler to the main queue.
                           dispatch_async(dispatch_get_main_queue(), ^(void) {
                             [self setUpPlaybackOfVideoAsset:asset withKeys:assetKeysToLoadAndTest];
                           });
                         }];
}

- (void)setUpPlaybackOfVideoAsset:(AVAsset*)asset withKeys:(NSArray*)keys
{
    // First test whether the values of each of the keys we need have been successfully loaded.
    for (NSString* key in keys)
    {
        NSError* error = nil;
        if ([asset statusOfValueForKey:key error:&error] == AVKeyValueStatusFailed)
        {
            const char* keyName = [key UTF8String];
            DAVA::Logger::FrameworkDebug("[MovieView] unable to retrieve key %s", keyName);
            playerState = eStateInitializedWithError;
            return;
        }
    }

    if (![asset isPlayable] || [asset hasProtectedContent])
    {
        DAVA::Logger::FrameworkDebug("[MovieView] asset is not playable or has protected content");
        playerState = eStateInitializedWithError;
        return;
    }

    if ([[asset tracksWithMediaType:AVMediaTypeVideo] count] == 0)
    {
        DAVA::Logger::FrameworkDebug("[MovieView] no video tracks");
        playerState = eStateInitializedWithError;
        return;
    }

    // We can play this asset.
    videoView = [[NSView alloc] init];
    [videoView setWantsLayer:YES];
    videoView.layer.backgroundColor = [[NSColor clearColor] CGColor];
    NSView* openGLView = static_cast<NSView*>(DAVA::Core::Instance()->GetNativeView());
    [openGLView addSubview:videoView];

    AVPlayerLayer* newPlayerLayer = [AVPlayerLayer playerLayerWithPlayer:videoPlayer];
    [newPlayerLayer setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];

    NSString* mode = nullptr;
    switch (scalingMode)
    {
    case DAVA::scalingModeAspectFit:
        mode = AVLayerVideoGravityResizeAspect;
        break;
    case DAVA::scalingModeAspectFill:
        mode = AVLayerVideoGravityResizeAspectFill;
        break;
    case DAVA::scalingModeFill:
        mode = AVLayerVideoGravityResize;
        break;
    default:
        break;
    }
    newPlayerLayer.videoGravity = mode;

    [[videoView layer] addSublayer:newPlayerLayer];

    // Create a new AVPlayerItem and make it our player's current item.
    AVPlayerItem* playerItem = [AVPlayerItem playerItemWithAsset:asset];
    videoDuration = CMTimeGetSeconds(playerItem.asset.duration);
    [videoPlayer replaceCurrentItemWithPlayerItem:playerItem];
    playerState = eStateInitializedOK;

    // Apply the cached states, if any.
    [self applyVideoRect];
    [self applyPlaybackState];
    [self applyVisible];
}

- (void)setRect:(const DAVA::Rect&)rect
{
    videoRect = rect;
    if (playerState == eStateInitializedOK)
    {
        [self applyVideoRect];
    }
}

- (void)setVisible:(bool)isVisible
{
    videoVisible = isVisible;
    if (playerState == eStateInitializedOK)
    {
        [self applyVisible];
    }
}

- (void)play
{
    [self setPlaybackState:ePlayback];
}

- (void)stop
{
    [self setPlaybackState:eStopped];
}

- (void)pause
{
    [self setPlaybackState:ePaused];
}

- (void)setPlaybackState:(MoviePlayerHelperPlaybackState)state
{
    playbackState = state;
    if (playerState == eStateInitializedOK)
    {
        [self applyPlaybackState];
    }
}

- (void)applyVideoRect
{
    DAVA::VirtualCoordinatesSystem* coordSystem = DAVA::VirtualCoordinatesSystem::Instance();

    // 1. map virtual to physical
    DAVA::Rect rect = coordSystem->ConvertVirtualToPhysical(videoRect);
    rect += coordSystem->GetPhysicalDrawOffset();
    rect.y = coordSystem->GetPhysicalScreenSize().dy - (rect.y + rect.dy);

    // 2. map physical to window
    NSView* openGLView = static_cast<NSView*>(DAVA::Core::Instance()->GetNativeView());
    NSRect controlRect = [openGLView convertRectFromBacking:NSMakeRect(rect.x, rect.y, rect.dx, rect.dy)];
    [videoView setFrame:controlRect];
}

- (void)applyPlaybackState
{
    double curPlayTime = CMTimeGetSeconds(videoPlayer.currentTime);
    bool videoAtEnd = curPlayTime == videoDuration;

    switch (playbackState)
    {
    case ePlayback:
        if (videoAtEnd)
        { // Rewind video to beginning to play again
            [videoPlayer seekToTime:CMTimeMakeWithSeconds(0.0, 1)];
        }
        [videoPlayer play];
        break;
    case eStopped:
        [videoPlayer pause];
        break;
    case ePaused:
        [videoPlayer pause];
    default:
        break;
    }
}

- (void)applyVisible
{
    [videoView setHidden:!videoVisible];
}

- (bool)isPlaying
{
    if (playerState == eStateInitializedOK)
    {
        return ([videoPlayer rate] != 0.0f);
    }
    return false;
}

@end

namespace DAVA
{
MovieViewControl::MovieViewControl()
{
    moviePlayerHelper = [[MoviePlayerHelper alloc] init];

    CoreMacOSPlatformBase* xcore = static_cast<CoreMacOSPlatformBase*>(Core::Instance());
    appMinimizedRestoredConnectionId = xcore->signalAppMinimizedRestored.Connect(this, &MovieViewControl::OnAppMinimizedRestored);
}

MovieViewControl::~MovieViewControl()
{
    CoreMacOSPlatformBase* xcore = static_cast<CoreMacOSPlatformBase*>(Core::Instance());
    xcore->signalAppMinimizedRestored.Disconnect(appMinimizedRestoredConnectionId);

    MoviePlayerHelper* helper = (MoviePlayerHelper*)moviePlayerHelper;
    [helper release];
}

void MovieViewControl::Initialize(const Rect& rect)
{
    SetRect(rect);
}

void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    NSURL* movieURL = [NSURL fileURLWithPath:[NSString stringWithCString:moviePath.GetAbsolutePathname().c_str() encoding:NSASCIIStringEncoding]];
    [(MoviePlayerHelper*)moviePlayerHelper loadMovie:movieURL scalingMode:params.scalingMode];
}

void MovieViewControl::SetRect(const Rect& rect)
{
    [(MoviePlayerHelper*)moviePlayerHelper setRect:rect];
}

void MovieViewControl::SetVisible(bool isVisible)
{
    [(MoviePlayerHelper*)moviePlayerHelper setVisible:isVisible];
}

void MovieViewControl::Play()
{
    [(MoviePlayerHelper*)moviePlayerHelper play];
}

void MovieViewControl::Stop()
{
    [(MoviePlayerHelper*)moviePlayerHelper stop];
}

void MovieViewControl::Pause()
{
    [(MoviePlayerHelper*)moviePlayerHelper pause];
}

void MovieViewControl::Resume()
{
    Play();
}

bool MovieViewControl::IsPlaying()
{
    return [(MoviePlayerHelper*)moviePlayerHelper isPlaying];
}

void MovieViewControl::OnAppMinimizedRestored(bool minimized)
{
    SetVisible(!minimized);
}

} // namespace DAVA
