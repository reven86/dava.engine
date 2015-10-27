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
#include "Version.h"
#include "Platform/DPIHelper.h"

using namespace DAVA;

void FrameworkDidLaunched()
{
    KeyedArchive * appOptions = new KeyedArchive();

    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);
    appOptions->SetBool("trackFont", true);

    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
    appOptions->SetString("title", DAVA::Format("DAVA Framework - QuickEd | %s-%s", DAVAENGINE_VERSION, APPLICATION_BUILD_VERSION));

    Size2i screenSize = DPIHelper::GetScreenSize();
    appOptions->SetInt32("width",  screenSize.dx);
	appOptions->SetInt32("height", screenSize.dy);
    VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(screenSize.dx, screenSize.dy);
    VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(screenSize.dx, screenSize.dy, "Gfx");

    Core::Instance()->SetOptions(appOptions);
    VirtualCoordinatesSystem::Instance()->EnableReloadResourceOnResize(false);

    GameCore * core = new GameCore();
    Core::SetApplicationCore(core);

    DynamicBufferAllocator::SetPageSize(1024 * 512);
}

void FrameworkWillTerminate()
{
    ApplicationCore* core = Core::GetApplicationCore();
    SafeRelease(core);
}
