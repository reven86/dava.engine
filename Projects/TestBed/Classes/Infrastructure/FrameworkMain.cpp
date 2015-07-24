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


#include "DAVAEngine.h"
#include "GameCore.h"
#include "Platform/DeviceInfo.h"
#include "Base/Platform.h"

using namespace DAVA;

const int32 WIDTH  = 1024;
const int32 HEIGHT = 768;

void FrameworkDidLaunched()
{
    
    int32 screenWidth = 0;
    int32 screenHeight = 0;

    KeyedArchive * appOptions = new KeyedArchive();
    
    appOptions->SetString(String("title"), String("TestBed"));
    
#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    appOptions->SetInt32("orientation", Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT);
    appOptions->SetInt32("renderer", Core::RENDERER_OPENGL_ES_3_0);

    screenWidth = Max(DeviceInfo::GetScreenInfo().width, DeviceInfo::GetScreenInfo().height);
    screenHeight = Min(DeviceInfo::GetScreenInfo().width, DeviceInfo::GetScreenInfo().height);
    appOptions->SetBool("iPhone_autodetectScreenScaleFactor", true);

    DAVA::VirtualCoordinatesSystem::Instance()->SetProportionsIsFixed(false);
#elif defined (__DAVAENGINE_WIN_UAP__)
    screenWidth = DeviceInfo::GetScreenInfo().width;
    screenHeight = DeviceInfo::GetScreenInfo().height;

    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);
#else
    screenWidth = WIDTH;
    screenHeight = HEIGHT;

    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);
#endif 
    
    appOptions->SetInt32("width", screenWidth);
    appOptions->SetInt32("height", screenHeight);
    
    DAVA::VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(screenWidth, screenHeight);
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(screenWidth, screenHeight, "Gfx");

    GameCore * core = new GameCore();
    DAVA::Core::SetApplicationCore(core);
    DAVA::Core::Instance()->SetOptions(appOptions);
}

void FrameworkWillTerminate()
{

}
