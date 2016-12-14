#if !defined(__DAVAENGINE_COREV2__)

#import "OpenGLView.h"
#include "DAVAEngine.h"
#include <ApplicationServices/ApplicationServices.h>

extern void FrameworkMain(int argc, char* argv[]);

uint32 ConvertNSModifiersToUIEvent(long modifiers);

using namespace DAVA;

@implementation OpenGLView

- (id)initWithFrame:(NSRect)frameRect
{
    NSLog(@"[CoreMacOSPlatform] OpenGLView Init");

    NSLog(@"Display bpp: %ld", [self displayBitsPerPixel:kCGDirectMainDisplay]);

    // Pixel Format Attributes for the View-based (non-FullScreen) NSOpenGLContext
    NSOpenGLPixelFormatAttribute attrs[] =
    {

      // Specifying "NoRecovery" gives us a context that cannot fall back to the software renderer.  This makes the View-based context a compatible with the fullscreen context, enabling us to use the "shareContext" feature to share textures, display lists, and other OpenGL objects between the two.
      NSOpenGLPFANoRecovery,

      // Attributes Common to FullScreen and non-FullScreen
      NSOpenGLPFAColorSize, static_cast<NSOpenGLPixelFormatAttribute>([self displayBitsPerPixel:kCGDirectMainDisplay]), //24,
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
    NSLog(@"[CoreMacOSPlatform] NSOpenGLView pixelFormat RendererID = %08x", static_cast<unsigned>(rendererID));

    self = [super initWithFrame:frameRect pixelFormat:pixelFormat];

    return self;
}

- (size_t)displayBitsPerPixel:(CGDirectDisplayID)displayId
{
    CGDisplayModeRef mode = CGDisplayCopyDisplayMode(displayId);
    size_t depth = 0;

    CFStringRef pixEnc = CGDisplayModeCopyPixelEncoding(mode);
    if (CFStringCompare(pixEnc, CFSTR(IO32BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
        depth = 32;
    else if (CFStringCompare(pixEnc, CFSTR(IO16BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
        depth = 16;
    else if (CFStringCompare(pixEnc, CFSTR(IO8BitIndexedPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
        depth = 8;

    CGDisplayModeRelease(mode);
    CFRelease(pixEnc);

    return depth;
}

- (void)dealloc
{
    [super dealloc];
}

- (BOOL)isOpaque
{
    return YES;
}

- (void)reshape
{
    if (Renderer::IsInitialized())
    {
        NSSize windowSize = [self frame].size;
        float32 backingScale = Core::Instance()->GetScreenScaleFactor();

        GLint backingSize[2] = { GLint(windowSize.width * backingScale), GLint(windowSize.height * backingScale) };
        CGLSetParameter([[self openGLContext] CGLContextObj], kCGLCPSurfaceBackingSize, backingSize);
        CGLUpdateContext([[self openGLContext] CGLContextObj]);

        float32 scale = DeviceInfo::GetScreenInfo().scale;
        Core::Instance()->WindowSizeChanged(windowSize.width, windowSize.height, scale, scale);
        Core::Instance()->SetNativeView(self);

        Core::Instance()->SystemProcessFrame();
    }

    [super reshape];
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

- (IBAction)copy:sender
{
}

- (IBAction)paste:sender
{
}

static Vector<DAVA::UIEvent> activeTouches;

void ConvertNSEventToUIEvent(NSOpenGLView* glview, NSEvent* curEvent, UIEvent& event, UIEvent::Phase phase)
{
    NSUInteger nsModifiers = curEvent.modifierFlags;
    uint32 davaModifiers = ConvertNSModifiersToUIEvent(nsModifiers);

    event.timestamp = [curEvent timestamp];
    event.phase = phase;
    event.modifiers = davaModifiers;
    if (InputSystem::Instance()->GetMouseDevice().IsPinningEnabled())
    {
        event.physPoint.x = [curEvent deltaX];
        event.physPoint.y = [curEvent deltaY];
    }
    else
    {
        NSPoint p = [curEvent locationInWindow];

        event.physPoint.x = p.x;
        event.physPoint.y = [glview frame].size.height - p.y;
    }

    if (DAVA::UIEvent::Phase::WHEEL == phase)
    {
        const uint32 rawScrollCoefficient = 10;
        DAVA::float32 rawScrollDeltaX([curEvent scrollingDeltaX]);
        DAVA::float32 rawScrollDeltaY([curEvent scrollingDeltaY]);

        // detect the wheel event device
        // http://stackoverflow.com/questions/13807616/mac-cocoa-how-to-differentiate-if-a-nsscrollwheel-event-is-from-a-mouse-or-trac
        if (NSEventPhaseNone != [curEvent momentumPhase] || NSEventPhaseNone != [curEvent phase])
        {
            event.device = DAVA::eInputDevices::TOUCH_PAD;
        }
        else
        {
            event.device = DAVA::eInputDevices::MOUSE;
            // Invert scroll directions back because MacOS do it by self when Shift pressed
            if (([curEvent modifierFlags] & NSShiftKeyMask) != 0)
            {
                std::swap(rawScrollDeltaX, rawScrollDeltaY);
            }
        }

        if (YES == [curEvent hasPreciseScrollingDeltas])
        {
            // touchpad or other precise device
            // sends integer values (-3, -1, 0, 1, 40 etc)
            event.wheelDelta.x = rawScrollDeltaX / rawScrollCoefficient;
            event.wheelDelta.y = rawScrollDeltaY / rawScrollCoefficient;
        }
        else
        {
            // simple mouse - sends float values from 0.1 for one wheel tick
            event.wheelDelta.x = rawScrollDeltaX * rawScrollCoefficient;
            event.wheelDelta.y = rawScrollDeltaY * rawScrollCoefficient;
        }
    }
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
        if (it->mouseButton == static_cast<eMouseButtons>(button))
        {
            isFind = true;
            ConvertNSEventToUIEvent(self, curEvent, (*it), phase);

            break;
        }
    }

    if (!isFind)
    {
        UIEvent newTouch;
        newTouch.mouseButton = static_cast<eMouseButtons>(button);
        newTouch.device = eInputDevices::MOUSE;

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
            if (it->mouseButton == static_cast<eMouseButtons>(button))
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

- (void)mouseDown:(NSEvent*)theEvent
{
    [self process:DAVA::UIEvent::Phase::BEGAN touch:theEvent];
}

- (void)scrollWheel:(NSEvent*)theEvent
{
    DAVA::UIEvent ev;

    ConvertNSEventToUIEvent(self, theEvent, ev, DAVA::UIEvent::Phase::WHEEL);

    UIControlSystem::Instance()->OnInput(&ev);
}

- (void)magnifyWithEvent:(NSEvent*)event
{
    DAVA::UIEvent ev;

    NSUInteger nsModifiers = event.modifierFlags;
    uint32 davaModifiers = ConvertNSModifiersToUIEvent(nsModifiers);
    ev.modifiers = davaModifiers;
    ev.device = DAVA::eInputDevices::TOUCH_PAD;
    ev.timestamp = [event timestamp];
    ev.gesture.dx = 0.f;
    ev.gesture.dy = 0.f;
    ev.gesture.magnification = [event magnification];
    ev.gesture.rotation = 0.f;
    ev.phase = DAVA::UIEvent::Phase::GESTURE;

    UIControlSystem::Instance()->OnInput(&ev);
}

- (void)rotateWithEvent:(NSEvent*)event
{
    DAVA::UIEvent ev;

    NSUInteger nsModifiers = event.modifierFlags;
    uint32 davaModifiers = ConvertNSModifiersToUIEvent(nsModifiers);
    ev.modifiers = davaModifiers;
    ev.device = DAVA::eInputDevices::TOUCH_PAD;
    ev.timestamp = [event timestamp];
    ev.gesture.dx = 0.f;
    ev.gesture.dy = 0.f;
    ev.gesture.magnification = 0.f;
    ev.gesture.rotation = [event rotation];
    ev.phase = DAVA::UIEvent::Phase::GESTURE;

    UIControlSystem::Instance()->OnInput(&ev);
}

- (void)swipeWithEvent:(NSEvent*)event
{
    DAVA::UIEvent ev;

    NSUInteger nsModifiers = event.modifierFlags;
    uint32 davaModifiers = ConvertNSModifiersToUIEvent(nsModifiers);
    ev.modifiers = davaModifiers;
    ev.device = DAVA::eInputDevices::TOUCH_PAD;
    ev.timestamp = [event timestamp];
    ev.gesture.dx = [event deltaX] * (-1.f);
    ev.gesture.dy = [event deltaY] * (-1.f);
    ev.gesture.magnification = 0.f;
    ev.gesture.rotation = 0.f;
    ev.phase = DAVA::UIEvent::Phase::GESTURE;

    UIControlSystem::Instance()->OnInput(&ev);
}

- (void)mouseMoved:(NSEvent*)theEvent
{
    [self process:DAVA::UIEvent::Phase::MOVE touch:theEvent];
}

- (void)mouseUp:(NSEvent*)theEvent
{
    [self process:DAVA::UIEvent::Phase::ENDED touch:theEvent];
}

- (void)mouseDragged:(NSEvent*)theEvent
{
    [self process:DAVA::UIEvent::Phase::ENDED touch:theEvent];
}

- (void)rightMouseDown:(NSEvent*)theEvent
{
    [self process:DAVA::UIEvent::Phase::ENDED touch:theEvent];
}
- (void)rightMouseDragged:(NSEvent*)theEvent
{
    [self process:DAVA::UIEvent::Phase::ENDED touch:theEvent];
}
- (void)rightMouseUp:(NSEvent*)theEvent
{
    [self process:DAVA::UIEvent::Phase::ENDED touch:theEvent];
}
- (void)otherMouseDown:(NSEvent*)theEvent
{
    [self process:DAVA::UIEvent::Phase::ENDED touch:theEvent];
}
- (void)otherMouseDragged:(NSEvent*)theEvent
{
    [self process:DAVA::UIEvent::Phase::ENDED touch:theEvent];
}
- (void)otherMouseUp:(NSEvent*)theEvent
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
    NSUInteger nsModifiers = event.modifierFlags;
    uint32 davaModifiers = ConvertNSModifiersToUIEvent(nsModifiers);

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
        ev.device = eInputDevices::KEYBOARD;
        ev.key = davaKey;
        ev.modifiers = davaModifiers;

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
        ev.device = eInputDevices::KEYBOARD;
        ev.keyChar = static_cast<char16>(ch);
        ev.modifiers = davaModifiers;

        UIControlSystem::Instance()->OnInput(&ev);
    }
    keyboard.OnKeyPressed(davaKey);
}

- (void)keyUp:(NSEvent*)event
{
    int32 keyCode = [event keyCode];
    InputSystem* input = InputSystem::Instance();
    KeyboardDevice& keyboard = input->GetKeyboard();

    DAVA::UIEvent ev;

    NSUInteger nsModifiers = event.modifierFlags;
    uint32 davaModifiers = ConvertNSModifiersToUIEvent(nsModifiers);
    ev.phase = DAVA::UIEvent::Phase::KEY_UP;
    ev.key = keyboard.GetDavaKeyForSystemKey(keyCode);
    ev.device = eInputDevices::KEYBOARD;
    ev.modifiers = davaModifiers;

    UIControlSystem::Instance()->OnInput(&ev);

    keyboard.OnKeyUnpressed(ev.key);
}

- (void)flagsChanged:(NSEvent*)event
{
    // TODO add support for simultanious keys presed or released
    int32 newModifers = [event modifierFlags];
    static int32 masks[] = { NSAlphaShiftKeyMask, NSShiftKeyMask, NSControlKeyMask, NSAlternateKeyMask, NSCommandKeyMask };
    static Key keyCodes[] = { Key::CAPSLOCK, Key::LSHIFT, Key::LCTRL, Key::LALT, Key::LCMD };

    InputSystem* input = InputSystem::Instance();
    KeyboardDevice& keyboard = input->GetKeyboard();
    NSUInteger nsModifiers = event.modifierFlags;
    uint32 davaModifiers = ConvertNSModifiersToUIEvent(nsModifiers);

    for (unsigned i = 0; i < 5; i++)
    {
        if ((oldModifersFlags & masks[i]) != (newModifers & masks[i]))
        {
            DAVA::UIEvent ev;
            ev.device = eInputDevices::KEYBOARD;
            ev.modifiers = davaModifiers;
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
                else if (ev.key == Key::LCMD)
                {
                    if ((newModifers & NX_DEVICERCMDKEYMASK) || (oldModifersFlags & NX_DEVICERCMDKEYMASK))
                    {
                        ev.key = Key::RCMD;
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

uint32 ConvertNSModifiersToUIEvent(long nsModifiers)
{
    uint32 resModifiers = 0;
    if ((nsModifiers & NSShiftKeyMask) == NSShiftKeyMask)
    {
        resModifiers |= UIEvent::Modifier::SHIFT_DOWN;
    }
    if ((nsModifiers & NSControlKeyMask) == NSControlKeyMask)
    {
        resModifiers |= UIEvent::Modifier::CONTROL_DOWN;
    }
    if ((nsModifiers & NSAlternateKeyMask) == NSAlternateKeyMask)
    {
        resModifiers |= UIEvent::Modifier::ALT_DOWN;
    }
    if ((nsModifiers & NSCommandKeyMask) == NSCommandKeyMask)
    {
        resModifiers |= UIEvent::Modifier::COMMAND_DOWN;
    }
    return resModifiers;
}

@end

#endif // !__DAVAENGINE_COREV2__
