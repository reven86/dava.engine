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


#import  <AppKit/AppKit.h>
#import "NPAPIPluginMacOS.h"

#include "NPAPICorePlatformMacOS.h"

//#include "Render/RenderManager.h"

#include <pwd.h>

// Relative path to NPAPI Internet Plugins for MacOS.
#define PATH_TO_INTERNET_PLUGINS @"Library/Internet Plug-Ins"

// Arguments passed to the NPAPI plugin instance.
#define NPAPI_PLUGIN_ARGUMENT_WIDTH @"width"
#define NPAPI_PLUGIN_ARGUMENT_HEIGHT @"height"
#define NPAPI_PLUGIN_ARGUMENT_BUNDLE_NAME @"bundlename"

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

@implementation NPAPIPluginMacOS

-(NPAPIPluginMacOS*) initWithNPP:(NPP)instance
{
	if (self = [super init])
	{
		self->npp = instance;
		self->npWindow = NULL;
		self->openGLLayer = NULL;
		self->appCore = NULL;

		self->pluginWidth = 0;
		self->pluginHeight = 0;
		self->bundlePath = NULL;
		
		self->hasFocused = NO;

		oldModifiersFlags = 0;
	}
	
	return self;
}

-(void) dealloc
{
	[self terminateFramework];

	[openGLLayer release];
	openGLLayer = NULL;
	
	[bundlePath release];
	bundlePath = NULL;
	
	[super dealloc];
}

-(NPError) npNew:(NPMIMEType)pluginType npMode:(uint16_t) mode npArgCount:(int16_t)argCount
	  npArgNames:(char*[])argNames npArgValues:(char*[]) argValues npSavedData:(NPSavedData*) savedData
{
	// Lookup for the parameters needed for the initialization.
	NSLog(@"npNew: Params count: %i", argCount);
	
	// Lookup for the params needed to initialize the plugin properly.
	for (int16_t i = 0; i < argCount; i ++)
	{
		NSString* argName = [NSString stringWithCString:argNames[i] encoding:NSASCIIStringEncoding];
		NSString* argValue = [NSString stringWithCString:argValues[i] encoding:NSASCIIStringEncoding];
		NSLog(@"npNew: Param #%i name is %@, value is %@", i, argName, argValue);
		
		if ([argName caseInsensitiveCompare:NPAPI_PLUGIN_ARGUMENT_WIDTH] == NSOrderedSame)
		{
			self->pluginWidth = [argValue integerValue];
		}

		if ([argName caseInsensitiveCompare:NPAPI_PLUGIN_ARGUMENT_HEIGHT] == NSOrderedSame)
		{
			self->pluginHeight = [argValue integerValue];
		}

		if ([argName caseInsensitiveCompare:NPAPI_PLUGIN_ARGUMENT_BUNDLE_NAME] == NSOrderedSame)
		{
			[self setBundlePath: argValue];
		}
	}

	// Verify all the required parameters are specified.
	bool allParametersSpecified = true;
	if (self->pluginHeight == 0)
	{
		[self logRequiredParameterNotSpecified: NPAPI_PLUGIN_ARGUMENT_HEIGHT];
		allParametersSpecified = false;
	}
	
	if (self->pluginWidth == 0)
	{
		[self logRequiredParameterNotSpecified: NPAPI_PLUGIN_ARGUMENT_WIDTH];
		allParametersSpecified = false;
	}
	
	if (self->bundlePath == NULL)
	{
		[self logRequiredParameterNotSpecified: NPAPI_PLUGIN_ARGUMENT_BUNDLE_NAME];
		allParametersSpecified = false;
	}

	// Yuri Coder, 2013/05/22. Do we need to pass these parameters to some other location?
	return allParametersSpecified ? NPERR_NO_ERROR : NPERR_INVALID_PARAM;
}

-(void) logRequiredParameterNotSpecified:(NSString*) paramName
{
	NSLog(@"npNew: The %@ parameter for NPAPI DAVA Framework plugin is required is not specified", paramName);
}

// Set the NPWindow.
-(NPError) npSetWindow:(NPWindow*) window
{
	self->npWindow = window;
	return NPERR_NO_ERROR;
}

// Handle the Cocoa event came from browser.
-(NPError) npHandleEvent:(NPCocoaEvent*) event
{
	switch (event->type)
	{
		case NPCocoaEventWindowFocusChanged:
		{
			return [self handleWindowFocusChanged: event->data.focus.hasFocus];
		}

		default:
		{
			// Key/mouse tracking is implemented here.
			[self parseEvent:event];
			return NPERR_NO_ERROR;
		}
	}
}

-(void) parseEvent:(NPCocoaEvent*)event
{
	switch (event->type)
	{
		case NPCocoaEventDrawRect:
			break;

		case NPCocoaEventMouseDown:
            [self processEvent:DAVA::UIEvent::Phase::BEGAN touch:event];
            break;

        case NPCocoaEventMouseUp:
            [self processEvent:DAVA::UIEvent::Phase::ENDED touch:event];
            break;

        case NPCocoaEventMouseMoved:
            [self processEvent:DAVA::UIEvent::Phase::MOVE touch:event];
            break;

        case NPCocoaEventMouseEntered:
            break;

        case NPCocoaEventMouseExited:
            break;

        case NPCocoaEventMouseDragged:
            [self processEvent:DAVA::UIEvent::Phase::DRAG touch:event];
            break;

        case NPCocoaEventKeyDown:
            [self keyDown:event];
            break;

        case NPCocoaEventKeyUp:
            [self keyUp:event];
			break;

		case NPCocoaEventFlagsChanged:
			[self flagsChanged:event];
			break;

		case NPCocoaEventScrollWheel:
			break;

		case NPCocoaEventTextInput:
			break;

		default:
			break;
	}
}

- (void)moveTouchesToVector:(NPCocoaEvent*)curEvent touchPhase:(DAVA::UIEvent::Phase)touchPhase outTouches:(DAVA::Vector<DAVA::UIEvent>*)outTouches
{
	int button = 0;
	button = curEvent->data.mouse.buttonNumber + 1;
	time_t timestamp = time(NULL);

    if (touchPhase == DAVA::UIEvent::Phase::DRAG)
    {
        for (DAVA::Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); it++)
        {
            NSPoint p;
            p.x = curEvent->data.mouse.pluginX;
            p.y = curEvent->data.mouse.pluginY;

			it->physPoint.x = p.x;
			it->physPoint.y = p.y;

            if (DAVA::InputSystem::Instance()->GetMouseCaptureMode() == DAVA::InputSystem::eMouseCaptureMode::PINING)
            {
                it->physPoint.x = curEvent->data.mouse.deltaX;
                it->physPoint.y = curEvent->data.mouse.deltaY;
            }

            it->tapCount = DAVA::Max(curEvent->data.mouse.clickCount, 1);
            it->timestamp = timestamp;
            it->phase = touchPhase;
        }
    }

	bool isFind = false;
	for(DAVA::Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); it++)
	{
		if(it->tid == button)
		{
			isFind = true;

			NSPoint p;
			p.x = curEvent->data.mouse.pluginX;
			p.y = curEvent->data.mouse.pluginY;

			it->physPoint.x = p.x;
			it->physPoint.y = p.y;

            if (DAVA::InputSystem::Instance()->GetMouseCaptureMode() == DAVA::InputSystem::eMouseCaptureMode::PINING)
            {
                it->physPoint.x = curEvent->data.mouse.deltaX;
                it->physPoint.y = curEvent->data.mouse.deltaY;
            }

            it->tapCount = curEvent->data.mouse.clickCount;
            it->timestamp = timestamp;
            it->phase = touchPhase;

            break;
        }
	}

	if(!isFind)
	{
		DAVA::UIEvent newTouch;
		newTouch.tid = button;
		NSPoint p;
		p.x = curEvent->data.mouse.pluginX;
		p.y = curEvent->data.mouse.pluginY;

		newTouch.physPoint.x = p.x;
		newTouch.physPoint.y = p.y;

        if (DAVA::InputSystem::Instance()->GetMouseCaptureMode() == DAVA::InputSystem::eMouseCaptureMode::PINING)
        {
            newTouch.physPoint.x = curEvent->data.mouse.deltaX;
            newTouch.physPoint.y = curEvent->data.mouse.deltaY;
        }

        newTouch.tapCount = curEvent->data.mouse.clickCount;
        newTouch.timestamp = timestamp;
        newTouch.phase = touchPhase;
        allTouches.push_back(newTouch);
    }

	for(DAVA::Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); it++)
	{
		outTouches->push_back(*it);
	}

    if (touchPhase == DAVA::UIEvent::Phase::ENDED || touchPhase == DAVA::UIEvent::Phase::MOVE)
    {
        for (DAVA::Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); it++)
        {
            if (it->tid == button)
            {
                allTouches.erase(it);
				break;
			}
		}
	}
}

- (void)processEvent:(DAVA::UIEvent::Phase)touchPhase touch:(NPCocoaEvent*)touch
{
	DAVA::Vector<DAVA::UIEvent> touches;
	[self moveTouchesToVector:touch touchPhase:touchPhase outTouches:&touches];

    DAVA::UIControlSystem::Instance()->OnInput(&touches[0]);
    touches.clear();
}

-(void) keyDown:(NPCocoaEvent*)event
{
	NSString* s = (NSString*)event->data.key.characters;
	unichar c = [s characterAtIndex:0];

    DAVA::InputSystem* input = DAVA::InputSystem::Instance();
    DAVA::KeyboardDevice& keyboard = input->GetKeyboard();
    DAVA::int32 keyCode = event->data.key.keyCode;

    time_t timestamp = time(NULL);

    DAVA::UIEvent ev;
    ev.keyChar = c;
    if (c == 0)
    {
        ev.phase = DAVA::UIEvent::Phase::KEY_DOWN;
    }
    else
    {
        ev.phase = DAVA::UIEvent::Phase::CHAR;
    }
    ev.timestamp = timestamp;
    ev.tapCount = 1;
    ev.tid = keyboard.GetDavaKeyForSystemKey(keyCode);

    DAVA::Vector<DAVA::UIEvent> touches;
    touches.push_back(ev);

    DAVA::UIControlSystem::Instance()->OnInput(&touches[0]);

    keyboard.OnSystemKeyPressed(keyCode);
}

-(void) keyUp:(NPCocoaEvent*) event
{
    NSString* s = (NSString*)event->data.key.characters;
    unichar c = [s characterAtIndex:0];

    DAVA::InputSystem* input = DAVA::InputSystem::Instance();
    DAVA::KeyboardDevice& keyboard = input->GetKeyboard();
    DAVA::int32 keyCode = event->data.key.keyCode;

    time_t timestamp = time(NULL);

    DAVA::UIEvent ev;
    ev.keyChar = c;
    if (c == 0)
    {
        ev.phase = DAVA::UIEvent::Phase::KEY_UP;
    }
    else
    {
        ev.phase = DAVA::UIEvent::Phase::CHAR;
    }
    ev.timestamp = timestamp;
    ev.tapCount = 1;
    ev.tid = keyboard.GetDavaKeyForSystemKey(keyCode);

    DAVA::Vector<DAVA::UIEvent> touches;
    touches.push_back(ev);

    DAVA::UIControlSystem::Instance()->OnInput(&touches[0]); // , allTouches

    keyboard.OnSystemKeyUnpressed(keyCode);
}

-(void) flagsChanged:(NPCocoaEvent*) event
{
	DAVA::int32 newModifiers = event->data.key.modifierFlags;

	static DAVA::int32 masks[] = {
		NSAlphaShiftKeyMask,
		NSShiftKeyMask,
		NSControlKeyMask,
		NSAlternateKeyMask,
		NSCommandKeyMask};

    static DAVA::int32 keyCodes[] = {
        DAVA::DVKEY_CAPSLOCK,
        DAVA::DVKEY_SHIFT,
        DAVA::DVKEY_CTRL,
        DAVA::DVKEY_ALT,
        DAVA::DVKEY_LWIN
    };

    for (int i = 0; i < 5; i++)
    {
        if ((oldModifiersFlags & masks[i]) != (newModifiers & masks[i]))
        {
            if (newModifiers & masks[i])
            {
				DAVA::InputSystem::Instance()->GetKeyboard().OnSystemKeyPressed(keyCodes[i]);
			}
			else
			{
				DAVA::InputSystem::Instance()->GetKeyboard().OnSystemKeyUnpressed(keyCodes[i]);
			}
		}
	}

	oldModifiersFlags = newModifiers;
}


// Called when the browser requests us to return a value.
-(NPError) npGetValue:(NPPVariable) variable withParam:(void*)param
{
	switch (variable)
	{
		case NPPVpluginCoreAnimationLayer:
		{
			[self createOpenGLLayer];

			*((CALayer **)param) = openGLLayer;
			return NPERR_NO_ERROR;
		}
			
		default:
		{
			return NPERR_GENERIC_ERROR;
		}
	}
}

-(void) setBundlePath:(NSString*) bundleName
{
	NSString* homeDir = NULL;
	struct passwd* pwd = getpwuid(getuid());
	if (pwd)
	{
		homeDir = [NSString stringWithCString:pwd->pw_dir encoding:NSASCIIStringEncoding];
	}

	if (homeDir)
	{
		self->bundlePath = [[[homeDir stringByAppendingPathComponent: PATH_TO_INTERNET_PLUGINS]
							 stringByAppendingPathComponent: bundleName] retain];
	}

	NSLog(@"Bundle Path is set to %@", self->bundlePath);
}

-(void) createOpenGLLayer
{
	if (self->openGLLayer)
	{
		// Already created.
		return;
	}

	NSLog(@"Creating NPAPI OpenGL Layer");
	openGLLayer = [[NPAPIOpenGLLayerMacOS alloc] initWithPluginInstance:self];
	openGLLayer.opaque = YES;
	
	// Initialize the layer with the height and width of the plugin.
	[openGLLayer setFrame:CGRectMake(0.0f,  0.0f, self->pluginWidth, self->pluginHeight)];


	// The OpenGL layer is async and will be responsible for updates by itself.
	openGLLayer.asynchronous = YES;
	[openGLLayer retain];   // should be balanced by a -release in NPP_Destroy
	
	// Ready to initialize the framework.
	[self initializeFramework];
}

-(void) initializeFramework
{
	//sleep(15);
	DAVA::NPAPICoreMacOSPlatform * core = new DAVA::NPAPICoreMacOSPlatform();
	core->CreateSingletons();

	#if defined (__DAVAENGINE_NPAPI__)
	// A Bundle Path have to be updated for the NPAPI Plugin.
	// Yuri Coder, 2013/05/22. TODO: looks like temporary patch.
	DAVA::FilePath::InitializeBundleNameNPAPI([bundlePath UTF8String]);
	#endif // #if defined (__DAVAENGINE_NPAPI__)

	FrameworkDidLaunched();
#if RHI_COMPLETE
    DAVA::RenderManager::Create(DAVA::Core::RENDERER_OPENGL);
#endif
    DAVA::RenderSystem2D::Instance()->Init();

	appCore = DAVA::Core::GetApplicationCore();
}

-(void) doInitializationOnFirstDraw
{
#if RHI_COMPLETE
    DAVA::RenderManager::Instance()->DetectRenderingCapabilities();
#endif

    NSRect rect = NSRectFromCGRect([openGLLayer frame]);
#if RHI_COMPLETE
    DAVA::RenderManager::Instance()->SetRenderContextId((uint64)CGLGetCurrentContext());
    DAVA::RenderManager::Instance()->Init(rect.size.width, rect.size.height);
#endif
    DAVA::VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(rect.size.width, rect.size.height);
    DAVA::VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(rect.size.width, rect.size.height);
    DAVA::VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(rect.size.width, rect.size.height);
	
	NSLog(@"[NPAPICoreMacOSPlatform] SystemAppStarted");
	DAVA::Core::Instance()->SystemAppStarted();
}

-(void) terminateFramework
{
	FrameworkWillTerminate();	
}

#pragma mark Event Handlers

-(NPError) handleWindowFocusChanged:(BOOL)newValue
{
	NSLog(@"NPAPI Plugin Focus Changed, new value is %i", newValue);
	
	if (newValue == hasFocused)
	{
		// No change actually happened.
		return NPERR_NO_ERROR;
	}

	hasFocused = newValue;
	if (newValue)
	{
		[self onResume];
	}
	else
	{
		[self onSuspend];
	}

	return NPERR_NO_ERROR;
}

- (void) onSuspend
{
    if(appCore)
    {
        appCore->OnSuspend();
    }
    else
    {
        DAVA::Core::Instance()->SetIsActive(false);
    }
}

- (void) onResume
{
    if(appCore)
    {
        appCore->OnResume();
    }
    else
    {
        DAVA::Core::Instance()->SetIsActive(true);
    }
}

@end
