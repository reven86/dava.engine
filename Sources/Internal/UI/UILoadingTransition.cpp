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


#include "UI/UILoadingTransition.h"
#include "Platform/SystemTimer.h"
#include "UI/UIControlSystem.h"
#include "UI/UIScreenManager.h"
#include "Debug/Replay.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/RenderHelper.h"
#include "Job/JobManager.h"

namespace DAVA
{
static const uint32 LOADING_THREAD_STACK_SIZE = 1024 * 1024; // 1 mb

UILoadingScreen::UILoadingScreen()
{
}

UILoadingScreen::~UILoadingScreen()
{
}

void UILoadingScreen::SetScreenToLoad(int32 screenId)
{
    nextScreenId = screenId;
    DVASSERT(!thread.Valid());
    thread = nullptr;
}

void UILoadingScreen::ThreadMessage(BaseObject* obj, void* userData, void* callerData)
{
    UIScreen* nextScreen = (nextScreenId != -1) ? UIScreenManager::Instance()->GetScreen(nextScreenId) : nullptr;
    if (nextScreen != nullptr)
    {
        nextScreen->LoadGroup();
    }
}

void UILoadingScreen::OnAppear()
{
    UIScreen::OnAppear();

    if (!thread)
    {
        UIControlSystem::Instance()->LockSwitch();
        UIControlSystem::Instance()->LockInput();

        thread = Thread::Create(Message(this, &UILoadingScreen::ThreadMessage));
        thread->SetStackSize(LOADING_THREAD_STACK_SIZE);
        thread->Start();
    }

    if (Replay::IsRecord() || Replay::IsPlayback())
    {
        Replay::Instance()->PauseReplay(true);
    }
}

void UILoadingScreen::Update(float32 timeElapsed)
{
    UIScreen::Update(timeElapsed);

    if ((thread) && (thread->GetState() == Thread::STATE_ENDED))
    {
        JobManager::Instance()->WaitMainJobs(thread->GetId());

        UIControlSystem::Instance()->UnlockInput();
        UIControlSystem::Instance()->UnlockSwitch();

        UIScreenManager::Instance()->SetScreen(nextScreenId);

        thread = nullptr;
    }
}

void UILoadingScreen::OnDisappear()
{
    UIScreen::OnDisappear();

    if (Replay::Instance())
    {
        Replay::Instance()->PauseReplay(false);
        SystemTimer::Instance()->SetFrameDelta(0.33f); //TODO: this is temporary solution for "first frame after loading" issue
    }
}
};
