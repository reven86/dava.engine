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

#include "UIUpdateSystem.h"
#include "Platform/SystemTimer.h"
#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "UI/Components/UIUpdateComponent.h"

namespace DAVA
{

UIUpdateSystem::UIUpdateSystem()
{
}

UIUpdateSystem::~UIUpdateSystem()
{
}

void UIUpdateSystem::Update()
{
    float32 timeElapsed = SystemTimer::FrameDelta();

    if (RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_UI_CONTROL_SYSTEM))
    {
        UIControl* currentScreen = UIControlSystem::Instance()->GetScreen();
        if (currentScreen)
        {
            SystemUpdate(currentScreen, timeElapsed);
        }

        UIControl* popupContainer = UIControlSystem::Instance()->GetPopupContainer();
        if (popupContainer)
        {
            SystemUpdate(popupContainer, timeElapsed);
        }

    }
}

void UIUpdateSystem::SystemUpdate(UIControl* control, float32 timeElapsed)
{
    UIControlSystem::Instance()->updateCounter++;

    UIUpdateComponent* component = control->GetComponent<UIUpdateComponent>();
    if (component != NULL && component->GetCustomUpdate() != (int)NULL)
    {
        component->GetCustomUpdate()(timeElapsed);
    }

    control->isUpdated = true;

    List<UIControl*>::iterator it = control->childs.begin();
    List<UIControl*>::iterator itEnd = control->childs.end();
    for (; it != itEnd; ++it)
    {
        (*it)->isUpdated = false;
    }

    it = control->childs.begin();
    while (it != itEnd)
    {
        control->isIteratorCorrupted = false;
        UIControl *current = *it;
        UIUpdateComponent* currComponent = current->GetComponent<UIUpdateComponent>();
        if (currComponent != NULL && currComponent->GetCustomNeedSystemUpdateCheck() != (int)NULL && !currComponent->GetCustomNeedSystemUpdateCheck()())
        {
            continue;
        }
        if (!current->isUpdated)
        {
            current->Retain();
            SystemUpdate(current, timeElapsed);
            current->Release();
            if (control->isIteratorCorrupted)
            {
                it = control->childs.begin();
                itEnd = control->childs.end();
                continue;
            }
        }
        ++it;
    }
}

}
