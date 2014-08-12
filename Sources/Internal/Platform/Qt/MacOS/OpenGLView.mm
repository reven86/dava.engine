/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#import "OpenGLView.h"
#include "DAVAEngine.h"

#include "Platform/Qt/QtLayer.h"

#if defined(__DAVAENGINE_MACOS__)


@implementation OpenGLView
@synthesize willQuit;

-(id) initWithFrame: (NSRect) frameRect
{
//	NSLog(@"[CoreMacOSPlatform] OpenGLView Init");
	
#ifdef __DAVAENGINE_MACOS_VERSION_10_6__
//	NSLog(@"Display bpp: %ld", [self displayBitsPerPixel:kCGDirectMainDisplay]);
#else //#ifdef __DAVAENGINE_MACOS_VERSION_10_6__
//	NSLog(@"Display bpp: %d", CGDisplayBitsPerPixel(kCGDirectMainDisplay));
#endif //#ifdef __DAVAENGINE_MACOS_VERSION_10_6__
	

    // Pixel Format Attributes for the View-based (non-FullScreen) NSOpenGLContext
    NSOpenGLPixelFormatAttribute attrs[] = 
	{
		
        // Specifying "NoRecovery" gives us a context that cannot fall back to the software renderer.  This makes the View-based context a compatible with the fullscreen context, enabling us to use the "shareContext" feature to share textures, display lists, and other OpenGL objects between the two.
        NSOpenGLPFANoRecovery,
		
        // Attributes Common to FullScreen and non-FullScreen
#ifdef __DAVAENGINE_MACOS_VERSION_10_6__
        NSOpenGLPFAColorSize, [self displayBitsPerPixel:kCGDirectMainDisplay],//24,
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
//    NSLog(@"[CoreMacOSPlatform] NSOpenGLView pixelFormat RendererID = %08x", (unsigned)rendererID);
	
    self = [super initWithFrame:frameRect pixelFormat:pixelFormat];
	if (self)
	{

	}
	trackingArea = nil;
	[self enableTrackingArea];
	isFirstDraw = true;
    
    [self setWantsBestResolutionOpenGLSurface:YES];

	// enable vsync
	GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
	
	DAVA::RenderManager::Instance()->SetRenderContextId((uint64)CGLGetCurrentContext());
	
//	activeCursor = 0;
    
    //RenderManager::Create(Core::RENDERER_OPENGL);
	
    willQuit = false;
    keyboardLocked = true;
    
    offset.x = 0;
    offset.y = 0;

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
    
    NSWindow *window = [self window];
    [window setAcceptsMouseMovedEvents:YES];

    trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:(NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInKeyWindow) owner:self userInfo:nil];
	[self addTrackingArea:trackingArea];
}

- (void) disableTrackingArea
{
	if (trackingArea)
    {
		[self removeTrackingArea: trackingArea];
    }
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
    NSRect boundRect = [self convertRectToBacking: rect];
    
    if(RenderManager::Instance())
        RenderManager::Instance()->Init(boundRect.size.width, boundRect.size.height);
    
    if(UIControlSystem::Instance())
        UIControlSystem::Instance()->SetInputScreenAreaSize(rect.size.width, rect.size.height);
    
    if(Core::Instance())
    {
        Core::Instance()->UnregisterAllAvailableResourceSizes();
        Core::Instance()->RegisterAvailableResourceSize(boundRect.size.width, boundRect.size.height, "Gfx");
        
        Core::Instance()->SetPhysicalScreenSize(boundRect.size.width, boundRect.size.height);
        Core::Instance()->SetVirtualScreenSize(rect.size.width, rect.size.height);
        Core::Instance()->CalculateScaleMultipliers();
    }
	
    isFirstDraw = true;
    
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
    if(willQuit || !QtLayer::Instance()->IsDAVAEngineEnabled())
        return;
    
	DAVA::RenderManager::Instance()->Lock();
	
	if (isFirstDraw)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		isFirstDraw = false;
	}	

	
	
	DAVA::Core::Instance()->SystemProcessFrame();

	
    DAVA::RenderManager::Instance()->SetColor(Color::White);
    
    
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
    [super resetCursorRects];
}

-(void)cursorUpdate:(NSEvent *)theEvent
{
	NSLog(@"OpenGLView  Cursor update");
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
//	return YES;
    return NO;
}



static Vector<DAVA::UIEvent> activeTouches;

void NSEventToUIEvent(NSEvent *nsEvent, DAVA::UIEvent &uiEvent, const NSRect &viewRect, const NSPoint &offset)
{
    int px = ([nsEvent locationInWindow].x + 0.6);
    int py = ([nsEvent locationInWindow].y + 0.6);

    // mosemove event comes from qt, so don't apply any offset to it
    if(nsEvent.type == NSMouseMoved)
    {
        uiEvent.physPoint.x = px;
        uiEvent.physPoint.y = viewRect.size.height - py;
    }
    else
    {
        uiEvent.physPoint.x = px - offset.x;
        uiEvent.physPoint.y = viewRect.size.height - (py - offset.y);
    }

    uiEvent.physPoint.x -= 3.0;
    uiEvent.physPoint.y -= 3.0;

//    printf("phys _ [%f][%f][%lu], offset [%f][%f]\n", uiEvent.physPoint.x, uiEvent.physPoint.y, nsEvent.type, offset.x, offset.y);

    uiEvent.timestamp = nsEvent.timestamp;
    uiEvent.tapCount = nsEvent.clickCount;
}

void MoveTouchsToVector(NSEvent *curEvent, int touchPhase, Vector<UIEvent> *outTouches, const NSRect &viewRect, const NSPoint &offset)
{
	int button = 0;
	if(curEvent.type == NSLeftMouseDown || curEvent.type == NSLeftMouseUp || curEvent.type == NSLeftMouseDragged || curEvent.type == NSMouseMoved)
	{
		button = 1;
	}
	else if(curEvent.type == NSRightMouseDown || curEvent.type == NSRightMouseUp || curEvent.type == NSRightMouseDragged)
	{
		button = 2;
	}
	else 
	{
		button = curEvent.buttonNumber + 1;
	}

//	NSLog(@"Event button %d", button);
	
	int phase = UIEvent::PHASE_MOVE;
	if(curEvent.type == NSLeftMouseDown || curEvent.type == NSRightMouseDown || curEvent.type == NSOtherMouseDown)
	{
		phase = UIEvent::PHASE_BEGAN;
//		NSLog(@"Event phase PHASE_BEGAN");
	}
	else if(curEvent.type == NSLeftMouseUp || curEvent.type == NSRightMouseUp || curEvent.type == NSOtherMouseUp)
	{
		phase = UIEvent::PHASE_ENDED;
//		NSLog(@"Event phase PHASE_ENDED");
	}
	else if(curEvent.type == NSLeftMouseDragged || curEvent.type == NSRightMouseDragged || curEvent.type == NSOtherMouseDragged)
	{
		phase = UIEvent::PHASE_DRAG;
	}
	
	if(phase == UIEvent::PHASE_DRAG)
	{
		for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
		{
            NSEventToUIEvent(curEvent, *it, viewRect, offset);
            it->phase = phase;
		}
	}
	
	bool isFind = false;
	for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
	{
		if(it->tid == button)
		{
			isFind = true;
			
            NSEventToUIEvent(curEvent, *it, viewRect, offset);
			it->phase = phase;

			break;
		}
	}
	
	if(!isFind)
	{
		UIEvent newTouch;
		newTouch.tid = button;

        NSEventToUIEvent(curEvent, newTouch, viewRect, offset);
		newTouch.phase = phase;
		activeTouches.push_back(newTouch);
	}

	for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
	{
		outTouches->push_back(*it);
	}

	if(phase == UIEvent::PHASE_ENDED || phase == UIEvent::PHASE_MOVE)
	{
		for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
		{
			if(it->tid == button)
			{
				activeTouches.erase(it);
				break;
			}
		}
	}
	
}

-(void)process:(int)touchPhase touch:(NSEvent*)touch
{
	Vector<DAVA::UIEvent> touches;
	Vector<DAVA::UIEvent> emptyTouches;
	MoveTouchsToVector(touch, touchPhase, &touches, [self frame], offset);
//	NSLog(@"----- Touches --------");
//	for(int i = 0; i < touches.size(); i++)
//	{
//		NSLog(@"Button %d       phase %d", touches[i].tid, touches[i].phase);
//	}
//	NSLog(@"----- ------- --------");
	UIControlSystem::Instance()->OnInput(touchPhase, emptyTouches, touches);
	touches.clear();
}

- (void) MouseMoved:(float32)x y:(float32)y
{
    NSRect sz = [self frame];
    NSEvent *theEvent = [NSEvent mouseEventWithType:NSMouseMoved
                        location:NSMakePoint(x, sz.size.height - y)
                        modifierFlags:0 timestamp:nil windowNumber:0 context:nil eventNumber:0 clickCount:1 pressure:0];

	[self process:DAVA::UIEvent::PHASE_MOVE touch:theEvent];
    
//    NSLog(@"----- MOVE --!@!--");
}

- (void)CalcOffset:(NSEvent *)theEvent
{
    NSPoint origPos = [self convertPointToBacking:[theEvent locationInWindow]];
    NSPoint p = [self convertPointFromBase:origPos];
    offset.x = [theEvent locationInWindow].x - p.x;
    offset.y = [theEvent locationInWindow].y - p.y;
    
    //printf("offset [%f, %f]\n", offset.x, offset.y);
}

- (void)mouseDown:(NSEvent *)theEvent
{
//    NSPoint p = theEvent.locationInWindow;
//    printf("click [%f, %f]\n", p.x, p.y);
    
    [self CalcOffset:theEvent];
	[super mouseDown:theEvent];
	[self process:DAVA::UIEvent::PHASE_BEGAN touch:theEvent];
}

- (void)mouseUp:(NSEvent *)theEvent
{
    [self CalcOffset:theEvent];
    [super mouseUp:theEvent];
    [self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    [self CalcOffset:theEvent];
    [super mouseDragged:theEvent];
	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}

/*
- (void)mouseEntered:(NSEvent *)theEvent
{
    [super mouseEntered:theEvent];
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
    [super mouseExited:theEvent];
    [NSCursor unhide];
//	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}
*/

- (void)rightMouseDown:(NSEvent *)theEvent
{
    [self CalcOffset:theEvent];
    [super rightMouseDown:theEvent];
	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}
- (void)rightMouseDragged:(NSEvent *)theEvent
{
    [self CalcOffset:theEvent];
    [super rightMouseDragged:theEvent];
	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}
- (void)rightMouseUp:(NSEvent *)theEvent
{
    [self CalcOffset:theEvent];
    [super rightMouseUp:theEvent];
	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}
- (void)otherMouseDown:(NSEvent *)theEvent
{
    [self CalcOffset:theEvent];
    [super otherMouseDown:theEvent];
	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}
- (void)otherMouseDragged:(NSEvent *)theEvent
{
    [self CalcOffset:theEvent];
    [super otherMouseDragged:theEvent];
	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}
- (void)otherMouseUp:(NSEvent *)theEvent
{
    [self CalcOffset:theEvent];
    [super otherMouseUp:theEvent];
	[self process:DAVA::UIEvent::PHASE_ENDED touch:theEvent];
}

static int32 oldModifersFlags = 0;
- (void) keyDown:(NSEvent *)event
{
    if(keyboardLocked)
    {
        {
            //		Logger::FrameworkDebug("glview keypress!");
            unichar c = [[event characters] characterAtIndex:0];
            
            Vector<DAVA::UIEvent> touches;
            Vector<DAVA::UIEvent> emptyTouches;
            
            for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
            {
                touches.push_back(*it);
            }
            
            DAVA::UIEvent ev;
            ev.keyChar = c;
            ev.phase = DAVA::UIEvent::PHASE_KEYCHAR;
            ev.timestamp = event.timestamp;
            ev.tapCount = 1;
            ev.tid = InputSystem::Instance()->GetKeyboard()->GetDavaKeyForSystemKey([event keyCode]);
            
            touches.push_back(ev);
            
            UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
            touches.pop_back();
            UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
        }
        
        InputSystem::Instance()->GetKeyboard()->OnSystemKeyPressed([event keyCode]);
        if ([event modifierFlags]&NSCommandKeyMask)
        {
            InputSystem::Instance()->GetKeyboard()->OnSystemKeyUnpressed([event keyCode]);
        }
    }

	[super keyDown:event];
}

- (void) keyUp:(NSEvent *)event
{
    if(keyboardLocked)
    {
        InputSystem::Instance()->GetKeyboard()->OnSystemKeyUnpressed([event keyCode]);
    }

	[super keyUp:event];
}

- (void) flagsChanged :(NSEvent *)event
{
    int32 newModifers = [event modifierFlags];
    static int32 masks[] = {NSAlphaShiftKeyMask, NSShiftKeyMask, NSControlKeyMask, NSAlternateKeyMask, NSCommandKeyMask};
    static int32 keyCodes[] = {DVMACOS_CAPS_LOCK, DVMACOS_SHIFT, DVMACOS_CONTROL, DVMACOS_OPTION, DVMACOS_COMMAND};

    for (int i = 0; i < 5; i++) 
    {
        if ((oldModifersFlags&masks[i]) != (newModifers&masks[i]))
        {
            if (newModifers&masks[i]) 
            {
                InputSystem::Instance()->GetKeyboard()->OnSystemKeyPressed(keyCodes[i]);
            }
            else 
            {
                InputSystem::Instance()->GetKeyboard()->OnSystemKeyUnpressed(keyCodes[i]);
            }
        }
    }
    
    
    oldModifersFlags = newModifers;
}

- (void) LockKeyboardInput:(bool) locked
{
    keyboardLocked = locked;
}



@end

#endif //#if defined(__DAVAENGINE_MACOS__)
