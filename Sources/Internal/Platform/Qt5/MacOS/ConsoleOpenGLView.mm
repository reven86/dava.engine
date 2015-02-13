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
#import "Platform/Qt5/MacOS/ConsoleOpenGLView.h"

#if defined(__DAVAENGINE_MACOS__)

@implementation ConsoleOpenGLView

-(id) initWithFrame: (NSRect) frameRect
{
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
//    NSLog(@"[CoreMacOSPlatform] NSOpenGLView pixelFormat RendererID = %08x", (unsigned)rendererID);
	
    self = [super initWithFrame:frameRect pixelFormat:pixelFormat];
	if (self)
	{

	}

    [self setWantsBestResolutionOpenGLSurface:YES];

	// enable vsync
	GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
	
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


@end

#endif //#if defined(__DAVAENGINE_MACOS__)
