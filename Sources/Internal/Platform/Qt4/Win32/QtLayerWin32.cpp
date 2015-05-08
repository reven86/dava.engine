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


#include "QtLayerWin32.h"

#if defined(__DAVAENGINE_WIN32__)

#include "Platform/Qt4/Win32/CorePlatformWin32Qt.h"
#include "Platform/DPIHelper.h"


extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

namespace DAVA 
{
    
QtLayerWin32::QtLayerWin32()
{
    WidgetCreated();
}

QtLayerWin32::~QtLayerWin32()
{
    AppFinished();
    WidgetDestroyed();
}
    
    
void QtLayerWin32::WidgetCreated()
{
}

void QtLayerWin32::WidgetDestroyed()
{
    
}
    
void QtLayerWin32::OnSuspend()
{
    SoundSystem::Instance()->Suspend();
//    Core::Instance()->SetIsActive(false);
}
    
void QtLayerWin32::OnResume()
{
    SoundSystem::Instance()->Resume();
    Core::Instance()->SetIsActive(true);
}
    
void QtLayerWin32::AppStarted()
{
    Core::Instance()->SystemAppStarted();
}

void QtLayerWin32::AppFinished()
{
    Core::Instance()->SystemAppFinished();
    FrameworkWillTerminate();
    Core::Instance()->ReleaseSingletons();
}


void QtLayerWin32::SetWindow(HINSTANCE hInstance, HWND hWindow, int32 width, int32 height)
{
	CoreWin32PlatformQt *core = static_cast<CoreWin32PlatformQt*>(CoreWin32PlatformQt::Instance());
	DVASSERT(core);

	core->SetupWindow(hInstance, hWindow);
	RenderManager::Create(Core::RENDERER_OPENGL);		
	RenderManager::Instance()->Create(hInstance, hWindow);
    RenderSystem2D::Instance()->Init();

	FrameworkDidLaunched();

	Resize(width, height);
	AppStarted();
}


void QtLayerWin32::Resize(int32 width, int32 height)
{
	RenderManager::Instance()->Init(width, height);
	VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(width, height);

    VirtualCoordinatesSystem::Instance()->UnregisterAllAvailableResourceSizes();
    VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(width, height, "Gfx");
    float64 screenScale = DPIHelper::GetDpiScaleFactor(0);
    if (screenScale != 1.0f)
    {
        VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize((int32)(width*screenScale), (int32)(height*screenScale), "Gfx2");
    }

    VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(width, height);
    VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(width, height);

    VirtualCoordinatesSystem::Instance()->ScreenSizeChanged();
}
    
void QtLayerWin32::Move(int32 x, int32 y)
{
    
}

    
void QtLayerWin32::ProcessFrame()
{
//    if(willQuit)
//        return;

//    if (activeCursor != RenderManager::Instance()->GetCursor())
//    {
//        activeCursor = RenderManager::Instance()->GetCursor();
//    }

//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    glClearColor(1.0f, 0.5f, 0.1f, 1.f);
    
    DAVA::RenderManager::Instance()->Lock();
    
//    if (isFirstDraw)
//    {
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// 
//        isFirstDraw = false;
//    }
    
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
//    if(DAVA::Core::Instance()->IsActive())
//    {
//        [[self openGLContext] flushBuffer];
//    }
    
    DAVA::RenderManager::Instance()->Unlock();   
}

void QtLayerWin32::LockKeyboardInput(bool locked)
{
	CoreWin32PlatformQt *core = static_cast<CoreWin32PlatformQt*>(CoreWin32PlatformQt::Instance());
	DVASSERT(core);

	core->SetFocused(locked);
}

};


#endif // #if defined(__DAVAENGINE_MACOS__)
