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
    
QtLayer::QtLayer()
    :   delegate(NULL)
    ,   isDAVAEngineEnabled(true)
{
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
    
void QtLayer::AppStarted()
{
    FrameworkDidLaunched();
    Core::Instance()->SystemAppStarted();
}

void QtLayer::AppFinished()
{
    Core::Instance()->SystemAppFinished();
    FrameworkWillTerminate();
    Core::Instance()->ReleaseSingletons();
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
    rhi::InvalidateCache(); //as QT itself can break gl states
    Core::Instance()->SystemProcessFrame();
}
    

void QtLayer::Resize(int32 width, int32 height)
{
    rhi::ResetParam resetParams;
    resetParams.width = width;
    resetParams.height = height;
    Renderer::Reset(resetParams);

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

    
void QtLayer::KeyPressed(char16 key, int32 count, uint64 timestamp)
{
    UIEvent ev;
    ev.keyChar = 0;
    ev.phase = UIEvent::PHASE_KEYCHAR;
    ev.timestamp = static_cast<float64>(timestamp);
    ev.tapCount = 1;
    ev.tid = key;

    UIControlSystem::Instance()->OnInput({ev}, allEvents);

    InputSystem::Instance()->GetKeyboard().OnKeyPressed(key);
}


void QtLayer::KeyReleased(char16 key)
{
    UIEvent ev;
    ev.keyChar = 0;
    ev.phase = UIEvent::PHASE_KEYCHAR_RELEASE;
    ev.tapCount = 1;
    ev.tid = key;

    UIControlSystem::Instance()->OnInput({ev}, allEvents);

    InputSystem::Instance()->GetKeyboard().OnKeyUnpressed(key);
}
    
    
void QtLayer::CopyEvents(DAVA::UIEvent &newEvent, const DAVA::UIEvent &sourceEvent)
{
    newEvent.tapCount = sourceEvent.tapCount;
    newEvent.point = sourceEvent.point;
    newEvent.physPoint = sourceEvent.physPoint;
    newEvent.timestamp = sourceEvent.timestamp;
    newEvent.phase = sourceEvent.phase;
}
    
void QtLayer::MoveTouchsToVector(const UIEvent &event, Vector<UIEvent> &outTouches)
{
    if(event.phase == UIEvent::PHASE_DRAG)
    {
        for (Vector<DAVA::UIEvent>::iterator it = allEvents.begin(); it != allEvents.end(); it++)
        {
            CopyEvents(*it, event);
        }
    }
    
    bool isFind = false;
    for (Vector<UIEvent>::iterator it = allEvents.begin(); it != allEvents.end(); it++)
    {
        if(it->tid == event.tid)
        {
            isFind = true;
            
            CopyEvents(*it, event);
            break;
        }
    }
    
    if(!isFind)
    {
        allEvents.push_back(event);
    }

    for (Vector<UIEvent>::iterator it = allEvents.begin(); it != allEvents.end(); it++)
    {
        outTouches.push_back(*it);
    }
    
    if(event.phase == UIEvent::PHASE_ENDED || event.phase == UIEvent::PHASE_MOVE)
    {
        for (Vector<DAVA::UIEvent>::iterator it = allEvents.begin(); it != allEvents.end(); it++)
        {
            if(it->tid == event.tid)
            {
                allEvents.erase(it);
                break;
            }
        }
    }
}
    
    
void QtLayer::MouseEvent(const UIEvent & event)
{
    Vector<UIEvent> touches;

    MoveTouchsToVector(event, touches);

    UIControlSystem::Instance()->OnInput(touches, allEvents);
    touches.clear();
}

    
#if defined (__DAVAENGINE_WIN32__)

void* QtLayer::CreateAutoreleasePool()
{
    return nullptr;
}

void QtLayer::ReleaseAutoreleasePool(void *pool)
{
    (void)pool;
}
    
#endif

};
