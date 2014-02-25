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

extern void FrameworkDidLaunched();

namespace DAVA 
{
    
QtLayerMacOS::QtLayerMacOS()
{
    WidgetCreated();
    AppStarted();
}

QtLayerMacOS::~QtLayerMacOS()
{
    AppFinished();
    WidgetDestroyed();
}
    
    
void QtLayerMacOS::WidgetCreated()
{
//	DisplayMode fullscreenMode = Core::Instance()->GetCurrentDisplayMode();
//	
	// launch framework and setup all preferences
    //TODO: maybe we need reorder calls 
    RenderManager::Create(Core::RENDERER_OPENGL_ES_2_0);
    FrameworkDidLaunched();
}

void QtLayerMacOS::WidgetDestroyed()
{
    
}
    
void QtLayerMacOS::OnSuspend()
{
    SoundSystem::Instance()->Suspend();
    Core::Instance()->SetIsActive(false);
}
    
void QtLayerMacOS::OnResume()
{
    SoundSystem::Instance()->Resume();
    Core::Instance()->SetIsActive(true);
}
    
void QtLayerMacOS::AppStarted()
{
    Core::Instance()->SystemAppStarted();
}




};


#endif // #if defined(__DAVAENGINE_MACOS__)
