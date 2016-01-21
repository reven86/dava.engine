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


#import "OpenGLView.h"
#include "DAVAEngine.h"
#include <ApplicationServices/ApplicationServices.h>

extern void FrameworkMain(int argc, char *argv[]);

@implementation OpenGLView
@synthesize willQuit;

-(id) initWithFrame: (NSRect) frameRect
{
	NSLog(@"[CoreMacOSPlatform] OpenGLView Init");
	
#ifdef __DAVAENGINE_MACOS_VERSION_10_6__
	NSLog(@"Display bpp: %ld", [self displayBitsPerPixel:kCGDirectMainDisplay]);
#else //#ifdef __DAVAENGINE_MACOS_VERSION_10_6__
	NSLog(@"Display bpp: %d", CGDisplayBitsPerPixel(kCGDirectMainDisplay));
#endif //#ifdef __DAVAENGINE_MACOS_VERSION_10_6__
	

    // Pixel Format Attributes for the View-based (non-FullScreen) NSOpenGLContext
    NSOpenGLPixelFormatAttribute attrs[] = 
	{
		
        // Specifying "NoRecovery" gives us a context that cannot fall back to the software renderer.  This makes the View-based context a compatible with the fullscreen context, enabling us to use the "shareContext" feature to share textures, display lists, and other OpenGL objects between the two.
        NSOpenGLPFANoRecovery,
		
        // Attributes Common to FullScreen and non-FullScreen
#ifdef __DAVAENGINE_MACOS_VERSION_10_6__
        NSOpenGLPFAColorSize, static_cast<NSOpenGLPixelFormatAttribute>([self displayBitsPerPixel:kCGDirectMainDisplay]),//24,
#else //#ifdef __DAVAENGINE_MACOS_VERSION_10_6__
        NSOpenGLPFAColorSize, CGDisplayBitsPerPixel(kCGDirectMainDisplay),//24,
#endif //#ifdef __DAVAENGINE_MACOS_VERSION_10_6__
        NSOpenGLPFADepthSize, 16,
        NSOpenGLPFAStencilSize, 8,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        0
    };
    GLint rendererID;
	
    // Create our non-FullScreen pixel format.
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	
    // Just as a diagnostic, report the renderer ID that this pixel format binds to.  CGLRenderers.h contains a list of known renderers and their corresponding RendererID codes.
    [pixelFormat getValues:&rendererID forAttribute:NSOpenGLPFARendererID forVirtualScreen:0];
    NSLog(@"[CoreMacOSPlatform] NSOpenGLView pixelFormat RendererID = %08x", (unsigned)rendererID);
	
    self = [super initWithFrame:frameRect pixelFormat:pixelFormat];
	trackingArea = nil;
	[self enableTrackingArea];
	isFirstDraw = true;

	// enable vsync
	GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    
    // enable retina resolution
    [self setWantsBestResolutionOpenGLSurface:YES];
	
#if RHI_COMPLETE
    DAVA::RenderManager::Instance()->SetRenderContextId((uint64)CGLGetCurrentContext());
#endif

    activeCursor = 0;

    willQuit = false;

    return self;
}

#ifdef __DAVAENGINE_MACOS_VERSION_10_6__
- (size_t) displayBitsPerPixel:(CGDirectDisplayID) displayId 
{
    
	CGDisplayModeRef mode = CGDisplayCopyDisplayMode(displayId);
	size_t depth = 0;
    
	CFStringRef pixEnc = CGDisplayModeCopyPixelEncoding(mode);
	if(CFStringCompare(pixEnc, CFSTR(IO32BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
		depth = 32;
	else if(CFStringCompare(pixEnc, CFSTR(IO16BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
		depth = 16;
	else if(CFStringCompare(pixEnc, CFSTR(IO8BitIndexedPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
		depth = 8;

    CGDisplayModeRelease(mode);
    CFRelease(pixEnc);

    return depth;
}
#endif //#ifdef __DAVAENGINE_MACOS_VERSION_10_6__


- (void) enableTrackingArea
{
	[trackingArea release];
	trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:(NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInKeyWindow) owner:self userInfo:nil];
	[self addTrackingArea:trackingArea];
}

- (void) disableTrackingArea
{
	if (trackingArea)
		[self removeTrackingArea: trackingArea];
}

- (void) dealloc
{
	
	[super dealloc];
}

- (BOOL)isOpaque
{
	return YES;
}

- (void)reshape
{
    if(Renderer::IsInitialized())
    {
        NSSize windowSize = self.frame.size;
        NSSize surfaceSize = [self convertRectToBacking:self.frame].size;
    
        float32 userScale = Core::Instance()->GetScreenScaleMultiplier();

        rhi::ResetParam params;
        params.width = surfaceSize.width;
        params.height = surfaceSize.height;
        params.scaleX = userScale;
        params.scaleY = userScale;
        Renderer::Reset(params);
        
        VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(windowSize.width, windowSize.height);
        VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(surfaceSize.width * userScale, surfaceSize.height * userScale);
        VirtualCoordinatesSystem::Instance()->ScreenSizeChanged();
        UIScreenManager::Instance()->ScreenSizeChanged();
    }
    
    [super reshape];
}

- (void)userFireTimer: (id)timer
{
	[self setNeedsDisplay:YES];
}
	
- (void)drawRect:(NSRect)theRect
{
    if(willQuit)
        return;
    
    DAVA::Core::Instance()->SystemProcessFrame();
}

- (void) resetCursorRects
{
	if (activeCursor)
	{
		NSCursor * cursor = (NSCursor*)activeCursor->GetMacOSXCursor();
		[self addCursorRect: [self bounds] cursor: cursor];
	}else {
		[super resetCursorRects];
	}
}

-(void)cursorUpdate:(NSEvent *)theEvent
{
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (BOOL)becomeFirstResponder
{
	return YES;
}

- (BOOL)resignFirstResponder
{	
	return YES;
}

static Vector<DAVA::UIEvent> activeTouches;

void ConvertNSEventToUIEvent(NSOpenGLView *glview, NSEvent* curEvent, UIEvent& event, UIEvent::Phase phase)
{
    NSPoint p = [curEvent locationInWindow];

    if (InputSystem::Instance()->GetMouseCaptureMode() == DAVA::InputSystem::eMouseCaptureMode::PINING)
    {
        event.physPoint.x = [curEvent deltaX];
        event.physPoint.y = [curEvent deltaY];
    }
    else
    {
        event.physPoint.x = p.x;
        event.physPoint.y = [glview frame].size.height - p.y;
    }

    if (DAVA::UIEvent::Phase::WHEEL != phase)
    {
        @try
        {
            event.tapCount = [curEvent clickCount];
        }
        @catch (NSException* exception)
        {
            String err([[NSString stringWithFormat:@"Error %@:", [exception reason]] UTF8String]);
            DVASSERT_MSG(false, DAVA::Format("You should not use clickCount property for that event type! %s", err.c_str()).c_str());
        }
    }

    if (DAVA::UIEvent::Phase::WHEEL == phase)
    {
        event.device = DAVA::UIEvent::Device::MOUSE;

        const uint32 rawScrollCoefficient = 10;

        DAVA::float32 rawScrollDelta([curEvent scrollingDeltaY]);
        if (YES == [curEvent hasPreciseScrollingDeltas])
        {
            // touchpad or other precise device
            // sends integer values (-3, -1, 0, 1, 40 etc)
            event.wheelDelta.y = rawScrollDelta / rawScrollCoefficient;
        }
        else
        {
            // simple mouse - sends float values from 0.1 for one wheel tick
            event.wheelDelta.y = rawScrollDelta * rawScrollCoefficient;
        }
    }

    event.timestamp = [curEvent timestamp];
    event.phase = phase;
}

- (void)moveTouchsToVector:(UIEvent::Phase)touchPhase curEvent:(NSEvent*)curEvent outTouches:(Vector<UIEvent>*)outTouches
{
    int button = 0;
    if (curEvent.type == NSLeftMouseDown || curEvent.type == NSLeftMouseUp || curEvent.type == NSLeftMouseDragged)
    {
        button = 1;
    }
    else if (curEvent.type == NSRightMouseDown || curEvent.type == NSRightMouseUp || curEvent.type == NSRightMouseDragged)
    {
        button = 2;
    }
    else if (curEvent.type != NSMouseMoved)
    {
        button = curEvent.buttonNumber + 1;
    }

    UIEvent::Phase phase = UIEvent::Phase::MOVE;
    if (curEvent.type == NSLeftMouseDown || curEvent.type == NSRightMouseDown || curEvent.type == NSOtherMouseDown)
    {
        phase = UIEvent::Phase::BEGAN;
    }
    else if (curEvent.type == NSLeftMouseUp || curEvent.type == NSRightMouseUp || curEvent.type == NSOtherMouseUp)
    {
        phase = UIEvent::Phase::ENDED;
    }
    else if (curEvent.type == NSLeftMouseDragged || curEvent.type == NSRightMouseDragged || curEvent.type == NSOtherMouseDragged)
    {
        phase = UIEvent::Phase::DRAG;
    }
    else if (curEvent.type == NSScrollWheel)
    {
        phase = UIEvent::Phase::WHEEL;
    }

    if (phase == UIEvent::Phase::DRAG)
    {
        for (Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); it++)
        {
            ConvertNSEventToUIEvent(self, curEvent, (*it), phase);
        }
    }

    bool isFind = false;
    for (Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); it++)
    {
        if (it->mouseButton == static_cast<UIEvent::MouseButton>(button))
        {
            isFind = true;
            ConvertNSEventToUIEvent(self, curEvent, (*it), phase);

            break;
        }
    }

    if (!isFind)
    {
        UIEvent newTouch;
        newTouch.mouseButton = static_cast<UIEvent::MouseButton>(button);
        newTouch.device = UIEvent::Device::MOUSE;

        ConvertNSEventToUIEvent(self, curEvent, newTouch, phase);

        allTouches.push_back(newTouch);
    }

    for (Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); it++)
    {
        outTouches->push_back(*it);
    }

    if (phase == UIEvent::Phase::ENDED || phase == UIEvent::Phase::MOVE)
    {
        for (Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); it++)
        {
            if (it->mouseButton == static_cast<UIEvent::MouseButton>(button))
            {
                allTouches.erase(it);
                break;
            }
        }
    }
}

- (void)process:(UIEvent::Phase)touchPhase touch:(NSEvent*)touch
{
    Vector<DAVA::UIEvent> touches;

    [self moveTouchsToVector:touchPhase curEvent:touch outTouches:&touches];

    for (auto& t : touches)
    {
        UIControlSystem::Instance()->OnInput(&t);
    }
    touches.clear();
}

- (void)mouseDown:(NSEvent *)theEvent
{
    [self process:DAVA::UIEvent::Phase::BEGAN touch:theEvent];
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    DAVA::UIEvent ev;

    ConvertNSEventToUIEvent(self, theEvent, ev, DAVA::UIEvent::Phase::WHEEL);

    UIControlSystem::Instance()->OnInput(&ev);
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    [self process:DAVA::UIEvent::Phase::MOVE touch:theEvent];
}

- (void)mouseUp:(NSEvent *)theEvent
{
    [self process:DAVA::UIEvent::Phase::ENDED touch:theEvent];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    [self process:DAVA::UIEvent::Phase::ENDED touch:theEvent];
}
- (void)mouseEntered:(NSEvent *)theEvent
{
    NSLog(@"mouse ENTERED");
#if RHI_COMPLETE
    if(RenderManager::Instance()->GetCursor())
    {
        if(RenderManager::Instance()->GetCursor()->IsShow())
            [NSCursor unhide];
        else
            [NSCursor hide];
    }
#endif
    //	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}
- (void)mouseExited:(NSEvent *)theEvent
{
    [NSCursor unhide];
}
- (void)rightMouseDown:(NSEvent *)theEvent
{
    [self process:DAVA::UIEvent::Phase::ENDED touch:theEvent];
}
- (void)rightMouseDragged:(NSEvent *)theEvent
{
    [self process:DAVA::UIEvent::Phase::ENDED touch:theEvent];
}
- (void)rightMouseUp:(NSEvent *)theEvent
{
    [self process:DAVA::UIEvent::Phase::ENDED touch:theEvent];
}
- (void)otherMouseDown:(NSEvent *)theEvent
{
    [self process:DAVA::UIEvent::Phase::ENDED touch:theEvent];
}
- (void)otherMouseDragged:(NSEvent *)theEvent
{
    [self process:DAVA::UIEvent::Phase::ENDED touch:theEvent];
}
- (void)otherMouseUp:(NSEvent *)theEvent
{
    [self process:DAVA::UIEvent::Phase::ENDED touch:theEvent];
}

static int32 oldModifersFlags = 0;

- (void)keyDown:(NSEvent*)event
{
    InputSystem* input = InputSystem::Instance();
    KeyboardDevice& keyboard = input->GetKeyboard();

    NSString* chars = [event characters];
    bool isRepeat = [event isARepeat];
    uint32 keyCode = [event keyCode];
    DAVA::Key davaKey = keyboard.GetDavaKeyForSystemKey(keyCode);
    uint32 charsLength = [chars length];

    // first key_down event to send
    {
        DAVA::UIEvent ev;
        if (isRepeat)
        {
            ev.phase = DAVA::UIEvent::Phase::KEY_DOWN_REPEAT;
        }
        else
        {
            ev.phase = DAVA::UIEvent::Phase::KEY_DOWN;
        }
        ev.device = UIEvent::Device::KEYBOARD;
        ev.key = davaKey;

        UIControlSystem::Instance()->OnInput(&ev);
    }
    // not send char event to be consistent with Windows
    for (uint32 i = 0; i < charsLength; ++i)
    {
        uint32 ch = [chars characterAtIndex:i];
        DVASSERT(ch < 0xFFFF);
        DAVA::UIEvent ev;
        if (isRepeat)
        {
            ev.phase = UIEvent::Phase::CHAR_REPEAT;
        }
        else
        {
            ev.phase = UIEvent::Phase::CHAR;
        }
        ev.device = UIEvent::Device::KEYBOARD;
        ev.keyChar = static_cast<char16>(ch);

        UIControlSystem::Instance()->OnInput(&ev);
    }
    keyboard.OnKeyPressed(davaKey);
}

- (void) keyUp:(NSEvent *)event
{
    int32 keyCode = [event keyCode];
    InputSystem* input = InputSystem::Instance();
    KeyboardDevice& keyboard = input->GetKeyboard();

    DAVA::UIEvent ev;

    ev.phase = DAVA::UIEvent::Phase::KEY_UP;
    ev.key = keyboard.GetDavaKeyForSystemKey(keyCode);
    ev.device = UIEvent::Device::KEYBOARD;

    UIControlSystem::Instance()->OnInput(&ev);

    keyboard.OnKeyUnpressed(ev.key);
}

- (void) flagsChanged :(NSEvent *)event
{
    // TODO add support for simultanious keys presed or released
    int32 newModifers = [event modifierFlags];
    static int32 masks[] = {NSAlphaShiftKeyMask, NSShiftKeyMask, NSControlKeyMask, NSAlternateKeyMask, NSCommandKeyMask};
    static Key keyCodes[] = { Key::CAPSLOCK, Key::LSHIFT, Key::LCTRL, Key::LALT, Key::LWIN };

    InputSystem* input = InputSystem::Instance();
    KeyboardDevice& keyboard = input->GetKeyboard();

    for (unsigned i = 0; i < 5; i++)
    {
        if ((oldModifersFlags & masks[i]) != (newModifers & masks[i]))
        {
            DAVA::UIEvent ev;
            ev.device = UIEvent::Device::KEYBOARD;

            ev.key = keyCodes[i];
            if (ev.key != Key::CAPSLOCK)
            {
                // determine right or left button
                if (ev.key == Key::LSHIFT)
                {
                    if ((newModifers & NX_DEVICERSHIFTKEYMASK) || (oldModifersFlags & NX_DEVICERSHIFTKEYMASK))
                    {
                        ev.key = Key::RSHIFT;
                    }
                }
                else if (ev.key == Key::LCTRL)
                {
                    if ((newModifers & NX_DEVICERCTLKEYMASK) || (oldModifersFlags & NX_DEVICERCTLKEYMASK))
                    {
                        ev.key = Key::RCTRL;
                    }
                }
                else if (ev.key == Key::LALT)
                {
                    if ((newModifers & NX_DEVICERALTKEYMASK) || (oldModifersFlags & NX_DEVICERALTKEYMASK))
                    {
                        ev.key = Key::RALT;
                    }
                }
                else if (ev.key == Key::LWIN)
                {
                    if ((newModifers & NX_DEVICERCMDKEYMASK) || (oldModifersFlags & NX_DEVICERCMDKEYMASK))
                    {
                        ev.key = Key::RWIN;
                    }
                }
            }

            if (newModifers & masks[i])
            {
                ev.phase = UIEvent::Phase::KEY_DOWN;

                UIControlSystem::Instance()->OnInput(&ev);

                keyboard.OnKeyPressed(ev.key);
            }
            else 
            {
                ev.phase = UIEvent::Phase::KEY_UP;

                UIControlSystem::Instance()->OnInput(&ev);

                keyboard.OnKeyUnpressed(ev.key);
            }
        }
    }
    
    
    oldModifersFlags = newModifers;
}

@end
