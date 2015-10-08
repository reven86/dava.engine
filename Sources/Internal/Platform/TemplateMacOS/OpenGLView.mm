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
	if (self)
	{

	}
	trackingArea = nil;
	[self enableTrackingArea];
	isFirstDraw = true;

	// enable vsync
	GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
	
	DAVA::RenderManager::Instance()->SetRenderContextId((uint64)CGLGetCurrentContext());
	
	activeCursor = 0;
    
    //RenderManager::Create(Core::RENDERER_OPENGL);
	
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
	NSRect rect = self.frame;
	DAVA::RenderManager::Instance()->Init(rect.size.width, rect.size.height);
	VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(rect.size.width, rect.size.height);
	VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(rect.size.width, rect.size.height);
	
	sizeChanged = YES;
	[super reshape];
}

- (void)userFireTimer: (id)timer
{
	[self setNeedsDisplay:YES];
}

//bool firstLaunch = true;
	
- (void)drawRect:(NSRect)theRect
{
    if(willQuit)
        return;
    
//	Logger::FrameworkDebug("drawRect started");
	
	if (activeCursor != RenderManager::Instance()->GetCursor())
	{
		activeCursor = RenderManager::Instance()->GetCursor();
		[[self window] invalidateCursorRectsForView: self];
	}
	
	
	DAVA::RenderManager::Instance()->Lock();
	
	if (isFirstDraw)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		isFirstDraw = false;
	}	
	

	
	DAVA::Core::Instance()->SystemProcessFrame();
	
/*	// This is an optimization.  If the view is being
	// resized, don't do a buffer swap.  The GL content
	// will be updated as part of the window flush anyway.
	// This makes live resize look nicer as the GL view
	// won't get flushed ahead of the window flush.  It also
	// makes live resize faster since we're not flushing twice.
	// Because I want the animtion to continue while resize
	// is happening, I use my own flag rather than calling
	// [self inLiveReize].  For most apps this wouldn't be
	// necessary.
 
	if(!sizeChanged)
	{
		[[self openGLContext] flushBuffer];
	}
	else glFlush();
	sizeChanged = NO; */
    if(DAVA::Core::Instance()->IsActive())
    {
        [[self openGLContext] flushBuffer];
    }
	DAVA::RenderManager::Instance()->Unlock();
//	Logger::FrameworkDebug("drawRect ended");

}

- (void) resetCursorRects
{
//	NSLog(@"OpenGLView resetCursorRects");
	//
	if (activeCursor)
	{
		//activeCursor->MacOSX_Set();
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

void ConvertNSEventToUIEvent(NSEvent *curEvent, UIEvent & event, int32 phase)
{
    NSPoint p = [curEvent locationInWindow];
    
    if(phase == UIEvent::PHASE_WHEEL)
    {
        event.physPoint.x = [curEvent scrollingDeltaX];
        event.physPoint.y = [curEvent scrollingDeltaY];
    }
    else if(InputSystem::Instance()->IsCursorPining())
    {
        event.physPoint.x = [curEvent deltaX];
        event.physPoint.y = [curEvent deltaY];
        
        event.tapCount = curEvent.clickCount;
    }
    else
    {
        event.physPoint.x = p.x;
        event.physPoint.y = VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy - p.y;
        
        event.tapCount = curEvent.clickCount;
    }
    event.timestamp = curEvent.timestamp;
    event.phase = phase;
}

- (void)moveTouchsToVector:(int)touchPhase curEvent:(NSEvent*)curEvent outTouches:(Vector<UIEvent>*)outTouches
{
	int button = 0;
	if(curEvent.type == NSLeftMouseDown || curEvent.type == NSLeftMouseUp || curEvent.type == NSLeftMouseDragged)
	{
		button = 1;
	}
	else if(curEvent.type == NSRightMouseDown || curEvent.type == NSRightMouseUp || curEvent.type == NSRightMouseDragged)
	{
		button = 2;
	}
	else if(curEvent.type != NSMouseMoved)
	{
		button = curEvent.buttonNumber + 1;
	}
	
	int phase = UIEvent::PHASE_MOVE;
	if(curEvent.type == NSLeftMouseDown || curEvent.type == NSRightMouseDown || curEvent.type == NSOtherMouseDown)
	{
		phase = UIEvent::PHASE_BEGAN;
	}
	else if(curEvent.type == NSLeftMouseUp || curEvent.type == NSRightMouseUp || curEvent.type == NSOtherMouseUp)
	{
		phase = UIEvent::PHASE_ENDED;
	}
	else if(curEvent.type == NSLeftMouseDragged || curEvent.type == NSRightMouseDragged || curEvent.type == NSOtherMouseDragged)
	{
		phase = UIEvent::PHASE_DRAG;
	}
	else if(curEvent.type == NSScrollWheel)
    {
        phase = UIEvent::PHASE_WHEEL;
    }
    
	if(phase == UIEvent::PHASE_DRAG)
	{
		for(Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); it++)
		{
            ConvertNSEventToUIEvent(curEvent, (*it), phase);
		}
	}
    
	bool isFind = false;
	for(Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); it++)
	{
		if(it->tid == button)
		{
			isFind = true;
			
            ConvertNSEventToUIEvent(curEvent, (*it), phase);

			break;
		}
	}
	
	if(!isFind)
	{
		UIEvent newTouch;
		newTouch.tid = button;
        
        ConvertNSEventToUIEvent(curEvent, newTouch, phase);
        
		allTouches.push_back(newTouch);
	}

	for(Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); it++)
	{
		outTouches->push_back(*it);
	}

	if(phase == UIEvent::PHASE_ENDED || phase == UIEvent::PHASE_MOVE)
	{
		for(Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); it++)
		{
			if(it->tid == button)
			{
				allTouches.erase(it);
				break;
			}
		}
	}
	
}


-(void)process:(int)touchPhase touch:(NSEvent*)touch
{
	Vector<DAVA::UIEvent> touches;

    [self moveTouchsToVector:touchPhase curEvent:touch outTouches:&touches];
    UIControlSystem::Instance()->OnInput(touches, allTouches);
    touches.clear();
}

- (void)mouseDown:(NSEvent *)theEvent
{
	[self process:DAVA::UIEvent::PHASE_BEGAN touch:theEvent];
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    DAVA::UIEvent ev;

    ev.phase = DAVA::UIEvent::PHASE_WHEEL;
    ev.deviceId = DAVA::UIEvent::PointerDeviceID::MOUSE;
    ev.physPoint.y = [theEvent scrollingDeltaY];

    UIControlSystem::Instance()->OnInput({ev}, allTouches);
}

- (void)mouseMoved:(NSEvent *)theEvent
{
	[self process:DAVA::UIEvent::PHASE_MOVE touch:theEvent];
}

- (void)mouseUp:(NSEvent *)theEvent
{
	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}
- (void)mouseEntered:(NSEvent *)theEvent
{
	NSLog(@"mouse ENTERED");
    if(RenderManager::Instance()->GetCursor())
    {
        if(RenderManager::Instance()->GetCursor()->IsShow())
            [NSCursor unhide];
        else
            [NSCursor hide];
    }
//	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}
- (void)mouseExited:(NSEvent *)theEvent
{
	NSLog(@"mouse EXITED");
    [NSCursor unhide];
//	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}
- (void)rightMouseDown:(NSEvent *)theEvent
{
	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}
- (void)rightMouseDragged:(NSEvent *)theEvent
{
	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}
- (void)rightMouseUp:(NSEvent *)theEvent
{
	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}
- (void)otherMouseDown:(NSEvent *)theEvent
{
	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}
- (void)otherMouseDragged:(NSEvent *)theEvent
{
	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}
- (void)otherMouseUp:(NSEvent *)theEvent
{
	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}

static int32 oldModifersFlags = 0;

- (void)keyDown:(NSEvent*)event
{
    InputSystem* input = InputSystem::Instance();
    KeyboardDevice& keyboard = input->GetKeyboard();

    NSString* chars = [event characters];
    bool isRepeat = [event isARepeat];
    int32 keyCode = [event keyCode];
    uint32 chars_length = [chars length];

    for (uint32 i = 0; i < chars_length; ++i)
    {
        uint32 ch = [chars characterAtIndex:i];
        DVASSERT(ch < 0xFFFF);
        DAVA::UIEvent ev;
        if (isRepeat)
        {
            ev.phase = UIEvent::PHASE_CHAR_REPEAT;
        }
        else
        {
            ev.phase = UIEvent::PHASE_CHAR;
        }
        ev.keyChar = static_cast<char16>(ch);

        UIControlSystem::Instance()->OnInput({ev}, allTouches);
    }

    if (keyCode > 0)
    {
        DAVA::UIEvent ev;
        if (isRepeat)
        {
            ev.phase = DAVA::UIEvent::PHASE_KEY_DOWN_REPEAT;
        }
        else
        {
            ev.phase = DAVA::UIEvent::PHASE_KEY_DOWN;
        }
        ev.tid = keyboard.GetDavaKeyForSystemKey(keyCode);

        UIControlSystem::Instance()->OnInput({ev}, allTouches);
    }

    keyboard.OnSystemKeyPressed(keyCode);
}

- (void) keyUp:(NSEvent *)event
{
    int32 keyCode = [event keyCode];
    InputSystem* input = InputSystem::Instance();
    KeyboardDevice& keyboard = input->GetKeyboard();

    DAVA::UIEvent ev;

    ev.phase = DAVA::UIEvent::PHASE_KEY_UP;
    ev.tid = keyboard.GetDavaKeyForSystemKey(keyCode);

    UIControlSystem::Instance()->OnInput({ev}, allTouches);

    keyboard.OnSystemKeyUnpressed(keyCode);
}

- (void) flagsChanged :(NSEvent *)event
{
    int32 newModifers = [event modifierFlags];
    static int32 masks[] = {NSAlphaShiftKeyMask, NSShiftKeyMask, NSControlKeyMask, NSAlternateKeyMask, NSCommandKeyMask};
    static int32 keyCodes[] = {DVMACOS_CAPS_LOCK, DVMACOS_SHIFT, DVMACOS_CONTROL, DVMACOS_OPTION, DVMACOS_COMMAND};

    for (int i = 0; i < 5; i++) 
    {
        if ((oldModifersFlags & masks[i]) != (newModifers & masks[i]))
        {
            DAVA::UIEvent ev;

            if (newModifers & masks[i])
            {
                ev.tid = keyCodes[i];
                ev.phase = UIEvent::PHASE_KEY_DOWN;

                UIControlSystem::Instance()->OnInput({ev}, allTouches);

                InputSystem::Instance()->GetKeyboard().OnSystemKeyPressed(keyCodes[i]);
            }
            else 
            {
                ev.tid = keyCodes[i];
                ev.phase = UIEvent::PHASE_KEY_UP;

                UIControlSystem::Instance()->OnInput({ev}, allTouches);

                InputSystem::Instance()->GetKeyboard().OnSystemKeyUnpressed(keyCodes[i]);
            }
        }
    }
    
    
    oldModifersFlags = newModifers;
}


@end
