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


#include "MovieViewControlMacOS.h"

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
    AVURLAsset* videoAsset;
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
}

// Set the video rectangle.
-(void) setRect:(const DAVA::Rect&) rect;

// Set the visible flag.
-(void) setVisible:(bool) isVisible;

// Load the movie in async way.
-(void) loadMovie:(NSURL*) movieURL;

// Playback control.
-(void) play;
-(void) pause;
-(void) stop;

// Whether the movie is playing?
-(bool) isPlaying;

@end

@implementation MoviePlayerHelper

-(MoviePlayerHelper*) init
{
    if (self = [super init])
    {
        videoAsset = nil;
        videoPlayer = nil;
        videoView = nil;

        playbackState = eNone;
        playerState = eStateNone;
        videoVisible = true;
    }
    
    return self;
}

-(void) dealloc
{
    [videoView removeFromSuperview];
    [videoView release];
    videoView = nil;

    [videoAsset release];
    videoAsset = nil;
    
    [videoPlayer release];
    videoPlayer = nil;

    [super dealloc];
}

-(void) loadMovie:(NSURL *)movieURL
{
    videoPlayer = [[AVPlayer alloc] init];
    videoAsset = [[AVAsset assetWithURL:movieURL] retain];

    playerState = eStateInitializing;
    NSArray *assetKeysToLoadAndTest = [NSArray arrayWithObjects:@"playable", @"tracks", @"duration", nil];
    [videoAsset loadValuesAsynchronouslyForKeys:assetKeysToLoadAndTest completionHandler:^(void)
    {
        // The asset invokes its completion handler on an arbitrary queue when loading is complete.
        // Because we want to access our AVPlayer in our ensuing set-up, we must dispatch our handler to the main queue.
        dispatch_async(dispatch_get_main_queue(), ^(void)
        {
            [self setUpPlaybackOfVideoAsset:videoAsset withKeys:assetKeysToLoadAndTest];
        });
    }];
}

- (void)setUpPlaybackOfVideoAsset:(AVAsset *)asset withKeys:(NSArray *)keys
{
    // First test whether the values of each of the keys we need have been successfully loaded.
    for (NSString *key in keys)
    {
        NSError *error = nil;
        if ([asset statusOfValueForKey:key error:&error] == AVKeyValueStatusFailed)
        {
            NSLog(@"MoviePlayerHelper: unable to retreive key %@", key);
            playerState = eStateInitializedWithError;
            return;
        }
    }
    
    if (![asset isPlayable] || [asset hasProtectedContent])
    {
        NSLog(@"MoviePlayerHelper: asset is not playable or has protected content!");
        playerState = eStateInitializedWithError;
        return;
    }

    if ([[asset tracksWithMediaType:AVMediaTypeVideo] count] == 0)
    {
        NSLog(@"MoviePlayerHelper: no Video Tracks found!");
        playerState = eStateInitializedWithError;
        return;
    }

    // We can play this asset.
    videoView = [[NSView alloc] init];
  	[videoView setWantsLayer:YES];
    videoView.layer.backgroundColor = [[NSColor clearColor] CGColor];
    NSView* openGLView = (NSView*)DAVA::Core::Instance()->GetNativeView();
    [openGLView addSubview:videoView];

    AVPlayerLayer *newPlayerLayer = [AVPlayerLayer playerLayerWithPlayer:videoPlayer];
    [newPlayerLayer setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
    [[videoView layer] addSublayer:newPlayerLayer];
    
    // Create a new AVPlayerItem and make it our player's current item.
    AVPlayerItem *playerItem = [AVPlayerItem playerItemWithAsset:asset];
    [videoPlayer replaceCurrentItemWithPlayerItem:playerItem];
    playerState = eStateInitializedOK;

    // Apply the cached states, if any.
    [self applyVideoRect];
    [self applyPlaybackState];
    [self applyVisible];
}

-(void) setRect:(const DAVA::Rect&)rect
{
    videoRect = rect;
    if (playerState == eStateInitializedOK)
    {
        [self applyVideoRect];
    }
}

-(void) setVisible:(bool) isVisible
{
    videoVisible = isVisible;
    if (playerState == eStateInitializedOK)
    {
        [self applyVisible];
    }
}

-(void) play
{
    [self setPlaybackState:ePlayback];
}

-(void) stop
{
    [self setPlaybackState:eStopped];
}

-(void) pause
{
    [self setPlaybackState:ePaused];
}

-(void) setPlaybackState:(MoviePlayerHelperPlaybackState) state
{
    playbackState = state;
    if (playerState == eStateInitializedOK)
    {
        [self applyPlaybackState];
    }
}

-(void) applyVideoRect
{
    NSRect movieViewRect = [videoView frame];

    DAVA::Rect convertedRect = DAVA::VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(videoRect);
    
    movieViewRect.size.width = convertedRect.dx;
    movieViewRect.size.height = convertedRect.dy;

    movieViewRect.origin.x = convertedRect.x;
    movieViewRect.origin.y = DAVA::VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy - convertedRect.y - convertedRect.dy;

    movieViewRect.origin.x += DAVA::VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset().x;
    movieViewRect.origin.y += DAVA::VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset().y;
    
    [videoView setFrame:movieViewRect];
}

-(void) applyPlaybackState
{
    if (!videoPlayer || playerState != eStateInitializedOK)
    {
        return;
    }

    switch (playbackState)
    {
        case ePlayback:
        {
            [videoPlayer play];
            break;
        }

        case eStopped:
        {
            [videoPlayer pause];
            [videoPlayer seekToTime:CMTimeMakeWithSeconds(0, 1)];
            break;
        }

        case ePaused:
        {
            [videoPlayer pause];
            break;
        }
        
        default:
        {
            break;
        }
    }
}

-(void) applyVisible
{
    if (!videoView)
    {
        return;
    }
    
    [videoView setHidden:!videoVisible];
}

-(bool) isPlaying
{
    if (!videoPlayer)
    {
        return false;
    }
    
    return ([videoPlayer rate] != 0.0f);
}

@end

namespace DAVA
{
MovieViewControl::MovieViewControl()
{
	moviePlayerHelper = [[MoviePlayerHelper alloc] init];
}
	
MovieViewControl::~MovieViewControl()
{
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
    [(MoviePlayerHelper*)moviePlayerHelper loadMovie:movieURL];
}
	
void MovieViewControl::SetRect(const Rect& rect)
{
    if (moviePlayerHelper)
    {
        [(MoviePlayerHelper*)moviePlayerHelper setRect:rect];
    }
}

void MovieViewControl::SetVisible(bool isVisible)
{
    if (moviePlayerHelper)
    {
        [(MoviePlayerHelper*)moviePlayerHelper setVisible:isVisible];
    }
}

void MovieViewControl::Play()
{
    if (moviePlayerHelper)
    {
        [(MoviePlayerHelper*)moviePlayerHelper play];
    }
}

void MovieViewControl::Stop()
{
    if (moviePlayerHelper)
    {
        [(MoviePlayerHelper*)moviePlayerHelper stop];
    }
}
	
void MovieViewControl::Pause()
{
    if (moviePlayerHelper)
    {
        [(MoviePlayerHelper*)moviePlayerHelper pause];
    }
}
	
void MovieViewControl::Resume()
{
	Play();
}
	
bool MovieViewControl::IsPlaying()
{
    if (!moviePlayerHelper)
    {
        return false;
    }
    
    return [(MoviePlayerHelper*)moviePlayerHelper isPlaying];
    /*
    if (!movieView)
    {
        return false;
    }

//    AVPlayer* player = [(AVPlayerView*)movieView player];
//    if (!player)
    {
        return false;
    }
    
//    return ([player rate] != 0.0f);
     */
     return true;
}

}