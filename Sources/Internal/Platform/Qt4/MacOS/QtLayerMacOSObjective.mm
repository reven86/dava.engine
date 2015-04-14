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


#include "QtLayerMacOS.h"


#if defined(__DAVAENGINE_MACOS__)

#import "AppKit/NSView.h"
#include "OpenGLView.h"

extern void FrameworkWillTerminate();


namespace DAVA 
{

NSView *qtNSView = NULL;
OpenGLView *openGLView = NULL;
    
void QtLayerMacOS::InitializeGlWindow(void *qtView, int32 width, int32 height)
{
    qtNSView = (NSView *)qtView;
    
    openGLView = [[OpenGLView alloc]initWithFrame: NSMakeRect(0, 0, width, height)];
    [qtNSView addSubview: openGLView];
    
	NSRect rect;
	rect.origin.x = 0;
	rect.origin.y = 0;
	rect.size.width = width;
	rect.size.height = height;
    
	[openGLView setFrame: rect];
}
    
    
void QtLayerMacOS::Resize(int32 width, int32 height)
{
  	NSRect rect;
	rect.origin.x = 0;
	rect.origin.y = 0;
	rect.size.width = width;
	rect.size.height = height;
 
    [openGLView setFrame: rect];
    
    [openGLView disableTrackingArea];
    [openGLView enableTrackingArea];
}
    
void  QtLayerMacOS::Move(int32 x, int32 y)
{
    // [openGLView setWindowOffset:Vector2((float32)x, (float32)y)];
}

    
void QtLayerMacOS::ProcessFrame()
{
    [openGLView setNeedsDisplay:YES];
}

void QtLayerMacOS::AppFinished()
{
    [openGLView removeFromSuperview];

    Core::Instance()->SystemAppFinished();
    FrameworkWillTerminate();
    Core::Instance()->ReleaseSingletons();
    
    [openGLView release];
}
    
void QtLayerMacOS::MouseMoved(float32 x, float32 y)
{
    [openGLView MouseMoved:x y: y];
}
    
void QtLayerMacOS::LockKeyboardInput(bool locked)
{
    [openGLView LockKeyboardInput: locked];
}

void * QtLayerMacOS::CreateAutoreleasePool()
{
    NSAutoreleasePool * autoreleasePool = [[NSAutoreleasePool alloc] init];
    return autoreleasePool;
}

void QtLayerMacOS::ReleaseAutoreleasePool(void *pool)
{
    NSAutoreleasePool * autoreleasePool = (NSAutoreleasePool *)pool;
    [autoreleasePool release];
}

void* QtLayerMacOS::GetOpenGLView()
{
	return openGLView;
}

};


#endif // #if defined(__DAVAENGINE_MACOS__)
