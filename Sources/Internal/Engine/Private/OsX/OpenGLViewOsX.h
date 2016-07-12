#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSOpenGLView.h>

#include "Engine/Private/EngineFwd.h"

// Subclass of NSOpenGLView
// Responsibilities:
//  - OpenGL-related tasks
//  - mouse and event forwarding to WindowOsXObjcBridge
@interface OpenGLViewOsX : NSOpenGLView
{
    DAVA::Private::WindowOsXObjcBridge* bridge;
}

- (id)initWithFrame:(NSRect)frameRect bridge:(DAVA::Private::WindowOsXObjcBridge*)objcBridge;

@end

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__