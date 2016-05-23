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

#include "Sound/SoundSystem.h"

#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

#include "UI/UIControlSystem.h"
#include <QSurfaceFormat>

extern void FrameworkWillTerminate();
extern void FrameworkDidLaunched();

namespace DAVA
{
QtLayer::QtLayer()
    : delegate(nullptr)
    , isDAVAEngineEnabled(true)
{
    QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
    fmt.setMajorVersion(3);
    fmt.setMinorVersion(3);
    fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
    QSurfaceFormat::setDefaultFormat(fmt);
}

QtLayer::~QtLayer()
{
    AppFinished();
}

void QtLayer::Quit()
{
    if (delegate)
    {
        delegate->Quit();
    }
}

void QtLayer::SetDelegate(QtLayerDelegate* delegate)
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
    Core::Instance()->Release();
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
    if (Core::Instance()->IsConsoleMode() == false)
    { // avoid calling of system process frame for console mode. Right not it is called from DAVA GL Widget. Will be refactored in future

        rhi::InvalidateCache(); //as QT itself can break gl states
        Core::Instance()->SystemProcessFrame();
    }
}

void QtLayer::Resize(int32 width, int32 height, float64 dpr)
{
    int32 realWidth = static_cast<int32>(width * dpr);
    int32 realHeight = static_cast<int32>(height * dpr);
    rhi::ResetParam resetParams;
    resetParams.width = realWidth;
    resetParams.height = realHeight;
    Renderer::Reset(resetParams);

    VirtualCoordinatesSystem* vcs = VirtualCoordinatesSystem::Instance();
    DVASSERT(nullptr != vcs)

    vcs->SetInputScreenAreaSize(realWidth, realHeight);

    vcs->UnregisterAllAvailableResourceSizes();
    vcs->RegisterAvailableResourceSize(width, height, "Gfx");
    vcs->RegisterAvailableResourceSize(width, height, "Gfx2");

    vcs->SetPhysicalScreenSize(realWidth, realHeight);
    vcs->SetVirtualScreenSize(width, height);
    vcs->ScreenSizeChanged();
}

void QtLayer::KeyPressed(Key key, uint64 timestamp)
{
    UIEvent ev;
    ev.phase = UIEvent::Phase::KEY_DOWN;
    ev.timestamp = static_cast<float64>(timestamp);
    ev.device = UIEvent::Device::KEYBOARD;
    ev.key = key;

    UIControlSystem::Instance()->OnInput(&ev);

    InputSystem::Instance()->GetKeyboard().OnKeyPressed(key);
}

void QtLayer::KeyReleased(Key key, uint64 timestamp)
{
    UIEvent ev;
    ev.phase = UIEvent::Phase::KEY_UP;
    ev.device = UIEvent::Device::KEYBOARD;
    ev.timestamp = static_cast<float64>(timestamp);
    ev.key = key;

    UIControlSystem::Instance()->OnInput(&ev);

    InputSystem::Instance()->GetKeyboard().OnKeyUnpressed(key);
}

void QtLayer::MouseEvent(const UIEvent& event)
{
    UIEvent evCopy(event);
    UIControlSystem::Instance()->OnInput(&evCopy);
}

    
#if defined(__DAVAENGINE_WIN32__)

void* QtLayer::CreateAutoreleasePool()
{
    return nullptr;
}

void QtLayer::ReleaseAutoreleasePool(void* pool)
{
    (void)pool;
}
    
#endif
};
