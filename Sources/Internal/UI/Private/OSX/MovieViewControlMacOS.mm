#include "UI/Private/OSX/MovieViewControlMacOS.h"

#if defined(__DAVAENGINE_MACOS__)
#if !defined(DISABLE_NATIVE_MOVIEVIEW)

#if defined(__DAVAENGINE_COREV2__)
#include "Engine/Engine.h"
#else
#include "Platform/TemplateMacOS/CorePlatformMacOS.h"
#endif
#include "Logger/Logger.h"

#include "UI/UIControlSystem.h"
#include "Platform/Steam.h"

#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <AppKit/AppKit.h>

#import "Engine/Mac/PlatformApi.h"

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
    
#if defined(__DAVAENGINE_COREV2__)
    DAVA::Window* window;
#endif
}

#if defined(__DAVAENGINE_COREV2__)
- (id)init:(DAVA::Window*)w;
#else
- (id)init;
#endif

// Set the video rectangle.
- (void)setRect:(const DAVA::Rect&)rect;

// Set the visible flag.
- (void)setVisible:(bool)isVisible;
- (bool)isVisible;

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

#if defined(__DAVAENGINE_COREV2__)
- (MoviePlayerHelper*)init:(DAVA::Window*)w
#else
- (id)init
#endif
{
    self = [super init];
    if (self != nullptr)
    {
#if defined(__DAVAENGINE_COREV2__)
        window = w;
#endif
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
#if defined(__DAVAENGINE_COREV2__)
    DAVA::PlatformApi::Mac::RemoveNSView(window, videoView);
#else
    [videoView removeFromSuperview];
#endif
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

#if defined(__DAVAENGINE_COREV2__)
    DAVA::PlatformApi::Mac::AddNSView(window, videoView);
#else
    NSView* openGLView = static_cast<NSView*>(DAVA::Core::Instance()->GetNativeView());
    [openGLView addSubview:videoView];
#endif

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
    videoRect.dx = std::max(0.0f, videoRect.dx);
    videoRect.dy = std::max(0.0f, videoRect.dy);
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

- (bool)isVisible
{
    return videoVisible;
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
    DAVA::Rect r = DAVA::UIControlSystem::Instance()->vcs->ConvertVirtualToInput(videoRect);
    DAVA::float32 dy = static_cast<DAVA::float32>(DAVA::UIControlSystem::Instance()->vcs->GetInputScreenSize().dy);
    [videoView setFrame:NSMakeRect(r.x, dy - r.y - r.dy, r.dx, r.dy)];
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
#if defined(__DAVAENGINE_COREV2__)
MovieViewControl::MovieViewControl(Window* w)
    : window(w)
#else
MovieViewControl::MovieViewControl()
#endif
{
#if defined(__DAVAENGINE_COREV2__)
    moviePlayerHelper = [[MoviePlayerHelper alloc] init:window];
    window->visibilityChanged.Connect(this, &MovieViewControl::OnWindowVisibilityChanged);
#else
    moviePlayerHelper = [[MoviePlayerHelper alloc] init];

    CoreMacOSPlatformBase* xcore = static_cast<CoreMacOSPlatformBase*>(Core::Instance());
    xcore->signalAppMinimizedRestored.Connect(this, &MovieViewControl::OnAppMinimizedRestored);
#endif

#if defined(__DAVAENGINE_STEAM__)
    Steam::GameOverlayActivated.Connect(this, &MovieViewControl::OnSteamOverlayChanged);
#endif
}

MovieViewControl::~MovieViewControl()
{
#if defined(__DAVAENGINE_STEAM__)
    Steam::GameOverlayActivated.Disconnect(this);
#endif

#if defined(__DAVAENGINE_COREV2__)
    window->visibilityChanged.Disconnect(this);
#else
    CoreMacOSPlatformBase* xcore = static_cast<CoreMacOSPlatformBase*>(Core::Instance());
    xcore->signalAppMinimizedRestored.Disconnect(this);
#endif

    MoviePlayerHelper* helper = static_cast<MoviePlayerHelper*>(moviePlayerHelper);
    [helper release];
}

void MovieViewControl::Initialize(const Rect& rect)
{
    SetRect(rect);
}

void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    NSURL* movieURL = [NSURL fileURLWithPath:[NSString stringWithCString:moviePath.GetAbsolutePathname().c_str() encoding:NSASCIIStringEncoding]];
    [static_cast<MoviePlayerHelper*>(moviePlayerHelper) loadMovie:movieURL scalingMode:params.scalingMode];
}

void MovieViewControl::SetRect(const Rect& rect)
{
    [static_cast<MoviePlayerHelper*>(moviePlayerHelper) setRect:rect];
}

void MovieViewControl::SetVisible(bool isVisible)
{
    [static_cast<MoviePlayerHelper*>(moviePlayerHelper) setVisible:isVisible];
}

#if defined(__DAVAENGINE_STEAM__)
void MovieViewControl::OnSteamOverlayChanged(bool overlayActivated)
{
    if (overlayActivated)
    {
        wasVisible = [static_cast<MoviePlayerHelper*>(moviePlayerHelper) isVisible];
        SetVisible(false);
    }
    else
    {
        SetVisible(wasVisible);
    }
}
#endif

void MovieViewControl::Play()
{
    [static_cast<MoviePlayerHelper*>(moviePlayerHelper) play];
}

void MovieViewControl::Stop()
{
    [static_cast<MoviePlayerHelper*>(moviePlayerHelper) stop];
}

void MovieViewControl::Pause()
{
    [static_cast<MoviePlayerHelper*>(moviePlayerHelper) pause];
}

void MovieViewControl::Resume()
{
    Play();
}

bool MovieViewControl::IsPlaying() const
{
    return [static_cast<MoviePlayerHelper*>(moviePlayerHelper) isPlaying];
}

#if defined(__DAVAENGINE_COREV2__)
void MovieViewControl::OnWindowVisibilityChanged(Window* w, bool visible)
{
    SetVisible(visible);
}
#else
void MovieViewControl::OnAppMinimizedRestored(bool minimized)
{
    SetVisible(!minimized);
}
#endif

} // namespace DAVA

#endif // !DISABLE_NATIVE_MOVIEVIEW
#endif // __DAVAENGINE_MACOS__
