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



#include "Platform/Qt5/QtLayer.h"

#include "Render/RenderManager.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#include "Platform/DPIHelper.h"

#include "Sound/SoundSystem.h"

#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

#include "UI/UIControlSystem.h"

extern void FrameworkWillTerminate();
extern void FrameworkDidLaunched();

namespace DAVA
{
    
void DVAssertMessage::InnerShow(eModalType /*modalType*/, const char* content)
{
    if(QtLayer::Instance())
        QtLayer::Instance()->ShowAsserMessage(content);
}
    
QtLayer::QtLayer()
    :   delegate(NULL)
    ,   isDAVAEngineEnabled(true)
{
    AppStarted();
}
    
QtLayer::~QtLayer()
{
    AppFinished();
}
    
    
void QtLayer::Quit()
{
    if(delegate)
    {
        delegate->Quit();
    }
}

void QtLayer::SetDelegate(QtLayerDelegate *delegate)
{
    this->delegate = delegate;
}

void QtLayer::ShowAsserMessage(const char * message)
{
    isDAVAEngineEnabled = false;
    
    if(delegate)
    {
        delegate->ShowAssertMessage(message);
    }
    
    isDAVAEngineEnabled = true;
}
    
    
void QtLayer::AppStarted()
{
    Core::Instance()->SystemAppStarted();
    
    RenderManager::Create(Core::RENDERER_OPENGL);
    FrameworkDidLaunched();
}

void QtLayer::AppFinished()
{
    Core::Instance()->SystemAppFinished();
    FrameworkWillTerminate();
    Core::Instance()->ReleaseSingletons();
#ifdef ENABLE_MEMORY_MANAGER
    if (MemoryManager::Instance() != 0)
    {
        MemoryManager::Instance()->FinalLog();
    }
#endif
}

    
void QtLayer::OnSuspend()
{
    SoundSystem::Instance()->Suspend();
    //    Core::Instance()->SetIsActive(false);
}

void QtLayer::OnResume()
{
    SoundSystem::Instance()->Resume();
    Core::Instance()->SetIsActive(true);
}

    
void QtLayer::ProcessFrame()
{
    RenderManager::Instance()->Lock();
    
    RenderManager::Instance()->SetColor(Color::White);
    Core::Instance()->SystemProcessFrame();
    
    RenderManager::Instance()->Unlock();
}
    
void QtLayer::InitializeGlWindow()
{
    RenderManager::Instance()->SetRenderContextId((uint64)CGLGetCurrentContext());
}


void QtLayer::Resize(int32 width, int32 height)
{
    RenderManager::Instance()->Init(width, height);
    RenderSystem2D::Instance()->Init();
    
    VirtualCoordinatesSystem *vcs = VirtualCoordinatesSystem::Instance();
    if(vcs)
    {
        vcs->SetInputScreenAreaSize(width, height);
        
        vcs->UnregisterAllAvailableResourceSizes();
        vcs->RegisterAvailableResourceSize(width, height, "Gfx");
        
        float64 screenScale = DPIHelper::GetDpiScaleFactor(0);
        if (screenScale != 1.0f)
        {
            vcs->RegisterAvailableResourceSize((int32)(width*screenScale), (int32)(height*screenScale), "Gfx2");
        }
        
        vcs->SetPhysicalScreenSize(width, height);
        vcs->SetVirtualScreenSize(width, height);
        vcs->ScreenSizeChanged();
    }
}

    
static Vector<UIEvent> activeTouches;

void QtLayer::KeyPressed(char16 key1, char16 davaKey, int32 count, uint64 timestamp)
{
    Vector<UIEvent> touches;
    Vector<UIEvent> emptyTouches;
    
    for(Vector<UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
    {
        touches.push_back(*it);
    }
    
    UIEvent ev;
    ev.keyChar = 0;
    ev.phase = UIEvent::PHASE_KEYCHAR;
    ev.timestamp = timestamp;
    ev.tapCount = 1;
    ev.tid = davaKey;
    
    touches.push_back(ev);
    
    UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
    touches.pop_back();
    UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
    
//    InputSystem::Instance()->GetKeyboard()->OnSystemKeyPressed(key);
    InputSystem::Instance()->GetKeyboard()->OnKeyPressed(davaKey);
}


void QtLayer::KeyReleased(char16 key, char16 davaKey)
{
    InputSystem::Instance()->GetKeyboard()->OnKeyUnpressed(davaKey);
//    InputSystem::Instance()->GetKeyboard()->OnSystemKeyUnpressed(key);
}
    
    
void MoveTouchsToVector(const DAVA::UIEvent &event, Vector<UIEvent> &outTouches)
{
    if(event.phase == UIEvent::PHASE_DRAG)
    {
        for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
        {
            it->physPoint = event.physPoint;
            it->phase = event.phase;
        }
    }
    
    
    bool isFind = false;
    for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
    {
        if(it->tid == event.tid)
        {
            isFind = true;

            it->physPoint = event.physPoint;
            it->phase = event.phase;
            break;
        }
    }
    
    if(!isFind)
    {
        activeTouches.push_back(event);
    }
    
    for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
    {
        outTouches.push_back(*it);
    }
    
    if(event.phase == UIEvent::PHASE_ENDED || event.phase == UIEvent::PHASE_MOVE)
    {
        for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
        {
            if(it->tid == event.tid)
            {
                activeTouches.erase(it);
                break;
            }
        }
    }
}
    
    
void QtLayer::MouseEvent(const UIEvent & event)
{
    Vector<UIEvent> touches;
    Vector<UIEvent> emptyTouches;

    MoveTouchsToVector(event, touches);
    
    UIControlSystem::Instance()->OnInput(event.phase, emptyTouches, touches);
}

    
#if defined (__DAVAENGINE_WIN32__)
void* QtLayer::CreateAutoreleasePool() {}
void QtLayer::ReleaseAutoreleasePool(void *pool) {}
#endif //

};
