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

#include "UI/UIControlSystem.h"
#include "UI/UIScreen.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "Logger/Logger.h"
#include "Render/OcclusionQuery.h"
#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"
#include "Debug/Replay.h"
#include "Debug/Stats.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "UI/Layouts/UILayoutSystem.h"
#include "Render/Renderer.h"
#include "Render/RenderHelper.h"
#include "UI/UIScreenshoter.h"
#include "Debug/Profiler.h"
#include "Render/2D/TextBlock.h"
#include "Platform/DPIHelper.h"
#include "Platform/DeviceInfo.h"
#include "Input/InputSystem.h"

namespace DAVA
{
const FastName FRAME_QUERY_UI_DRAW("OcclusionStatsUIDraw");

UIControlSystem::UIControlSystem()
{
    baseGeometricData.position = Vector2(0, 0);
    baseGeometricData.size = Vector2(0, 0);
    baseGeometricData.pivotPoint = Vector2(0, 0);
    baseGeometricData.scale = Vector2(1.0f, 1.0f);
    baseGeometricData.angle = 0;

    layoutSystem = new UILayoutSystem();
    styleSheetSystem = new UIStyleSheetSystem();
    screenshoter = new UIScreenshoter();

    popupContainer.Set(new UIControl(Rect(0, 0, 1, 1)));
    popupContainer->SetName("UIControlSystem_popupContainer");
    popupContainer->SetInputEnabled(false);
    popupContainer->InvokeActive(UIControl::eViewState::VISIBLE);

    // calculate default radius
    if (DeviceInfo::IsHIDConnected(DeviceInfo::eHIDType::HID_TOUCH_TYPE))
    {
        //half an inch
        defaultDoubleClickRadiusSquared = DPIHelper::GetScreenDPI() / 4;
        defaultDoubleClickRadiusSquared *= defaultDoubleClickRadiusSquared;
    }
    else
    {
        defaultDoubleClickRadiusSquared = 4; // default, if touch didn't detect, 4 - default pixels in windows desktop
    }
    doubleClickTime = defaultDoubleClickTime;
    doubleClickRadiusSquared = defaultDoubleClickRadiusSquared;
}

UIControlSystem::~UIControlSystem()
{
    popupContainer->InvokeInactive();
    popupContainer = nullptr;

    if (currentScreen.Valid())
    {
        currentScreen->InvokeInactive();
        currentScreen = nullptr;
    }

    SafeDelete(styleSheetSystem);
    SafeDelete(layoutSystem);
    SafeDelete(screenshoter);
}

void UIControlSystem::SetScreen(UIScreen* _nextScreen, UIScreenTransition* _transition)
{
    if (_nextScreen == currentScreen)
    {
        if (nextScreen != nullptr)
        {
            nextScreenTransition = nullptr;
            nextScreen = nullptr;
        }
        return;
    }

    if (nextScreen.Valid())
    {
        Logger::Warning("2 screen switches during one frame.");
    }

    nextScreenTransition = _transition;
    nextScreen = _nextScreen;

    if (nextScreen == nullptr)
    {
        removeCurrentScreen = true;
        ProcessScreenLogic();
    }
}

UIScreen* UIControlSystem::GetScreen() const
{
    return currentScreen.Get();
}

void UIControlSystem::AddPopup(UIPopup* newPopup)
{
    Set<UIPopup*>::const_iterator it = popupsToRemove.find(newPopup);
    if (popupsToRemove.end() != it)
    {
        popupsToRemove.erase(it);
        return;
    }

    if (newPopup->GetRect() != fullscreenRect)
    {
        newPopup->SystemScreenSizeChanged(fullscreenRect);
    }

    newPopup->LoadGroup();
    popupContainer->AddControl(newPopup);
}

void UIControlSystem::RemovePopup(UIPopup* popup)
{
    if (popupsToRemove.count(popup))
    {
        Logger::Warning("[UIControlSystem::RemovePopup] attempt to double remove popup during one frame.");
        return;
    }

    const List<UIControl*>& popups = popupContainer->GetChildren();
    if (popups.end() == std::find(popups.begin(), popups.end(), DynamicTypeCheck<UIPopup*>(popup)))
    {
        Logger::Error("[UIControlSystem::RemovePopup] attempt to remove uknown popup.");
        DVASSERT(false);
        return;
    }

    popupsToRemove.insert(popup);
}

void UIControlSystem::RemoveAllPopups()
{
    popupsToRemove.clear();
    const List<UIControl*>& totalChilds = popupContainer->GetChildren();
    for (List<UIControl*>::const_iterator it = totalChilds.begin(); it != totalChilds.end(); it++)
    {
        popupsToRemove.insert(DynamicTypeCheck<UIPopup*>(*it));
    }
}

UIControl* UIControlSystem::GetPopupContainer() const
{
    return popupContainer.Get();
}

UIScreenTransition* UIControlSystem::GetScreenTransition() const
{
    return currentScreenTransition.Get();
}

void UIControlSystem::Reset()
{
    SetScreen(nullptr);
}

void UIControlSystem::ProcessScreenLogic()
{
    /*
	 if next screen or we need to removecurrent screen
	 */
    if (screenLockCount == 0 && (nextScreen.Valid() || removeCurrentScreen))
    {
        RefPtr<UIScreen> nextScreenProcessed;
        RefPtr<UIScreenTransition> nextScreenTransitionProcessed;

        nextScreenProcessed = nextScreen;
        nextScreenTransitionProcessed = nextScreenTransition;
        nextScreen = nullptr; // functions called by this method can request another screen switch (for example, LoadResources)
        nextScreenTransition = nullptr;

        LockInput();

        CancelAllInputs();

        NotifyListenersWillSwitch(nextScreenProcessed.Get());

        if (nextScreenTransitionProcessed)
        {
            if (nextScreenTransitionProcessed->GetRect() != fullscreenRect)
            {
                nextScreenTransitionProcessed->SystemScreenSizeChanged(fullscreenRect);
            }

            nextScreenTransitionProcessed->StartTransition();
            nextScreenTransitionProcessed->SetSourceScreen(currentScreen.Get());
        }
        // if we have current screen we call events, unload resources for it group
        if (currentScreen)
        {
            currentScreen->InvokeInactive();

            RefPtr<UIScreen> prevScreen = currentScreen;
            currentScreen = nullptr;

            if ((nextScreenProcessed == nullptr) || (prevScreen->GetGroupId() != nextScreenProcessed->GetGroupId()))
            {
                prevScreen->UnloadGroup();
            }
        }
        // if we have next screen we load new resources, if it equal to zero we just remove screen
        if (nextScreenProcessed)
        {
            if (nextScreenProcessed->GetRect() != fullscreenRect)
            {
                nextScreenProcessed->SystemScreenSizeChanged(fullscreenRect);
            }

            nextScreenProcessed->LoadGroup();
        }
        currentScreen = nextScreenProcessed;
        if (currentScreen)
        {
            currentScreen->InvokeActive(UIControl::eViewState::VISIBLE);
        }

        NotifyListenersDidSwitch(currentScreen.Get());

        if (nextScreenTransitionProcessed)
        {
            nextScreenTransitionProcessed->SetDestinationScreen(currentScreen.Get());

            LockSwitch();
            LockInput();

            currentScreenTransition = nextScreenTransitionProcessed;
            currentScreenTransition->InvokeActive(UIControl::eViewState::VISIBLE);
        }

        UnlockInput();

        frameSkip = FRAME_SKIP;
        removeCurrentScreen = false;
    }
    else
    if (currentScreenTransition)
    {
        if (currentScreenTransition->IsComplete())
        {
            currentScreenTransition->InvokeInactive();

            RefPtr<UIScreenTransition> prevScreenTransitionProcessed = currentScreenTransition;
            currentScreenTransition = nullptr;

            UnlockInput();
            UnlockSwitch();

            prevScreenTransitionProcessed->EndTransition();
        }
    }

    /*
	 if we have popups to remove, we removes them here
	 */
    for (Set<UIPopup*>::iterator it = popupsToRemove.begin(); it != popupsToRemove.end(); it++)
    {
        UIPopup* p = *it;
        if (p)
        {
            p->Retain();
            popupContainer->RemoveControl(p);
            p->UnloadGroup();
            p->Release();
        }
    }
    popupsToRemove.clear();
}

void UIControlSystem::Update()
{
    TIME_PROFILE("UIControlSystem::Update");

    updateCounter = 0;
    ProcessScreenLogic();

    float32 timeElapsed = SystemTimer::FrameDelta();

    if (Renderer::GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_UI_CONTROL_SYSTEM))
    {
        if (currentScreenTransition)
        {
            currentScreenTransition->SystemUpdate(timeElapsed);
        }
        else if (currentScreen)
        {
            currentScreen->SystemUpdate(timeElapsed);
        }

        popupContainer->SystemUpdate(timeElapsed);
    }

    //Logger::Info("UIControlSystem::updates: %d", updateCounter);
}

void UIControlSystem::Draw()
{
    resizePerFrame = 0;
    TIME_PROFILE("UIControlSystem::Draw");

    TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "UIControlSystem::Draw")

    FrameOcclusionQueryManager::Instance()->BeginQuery(FRAME_QUERY_UI_DRAW);

    drawCounter = 0;

    const RenderSystem2D::RenderTargetPassDescriptor& descr = RenderSystem2D::Instance()->GetMainTargetDescriptor();

    if (descr.clearTarget)
    {
        rhi::Viewport viewport;
        viewport.x = viewport.y = 0U;
        viewport.width = descr.width == 0 ? static_cast<uint32>(Renderer::GetFramebufferWidth()) : descr.width;
        viewport.height = descr.height == 0 ? static_cast<uint32>(Renderer::GetFramebufferHeight()) : descr.height;
        const RenderSystem2D::RenderTargetPassDescriptor& descr = RenderSystem2D::Instance()->GetActiveTargetDescriptor();
        RenderHelper::CreateClearPass(descr.colorAttachment, descr.depthAttachment, descr.priority + PRIORITY_CLEAR, descr.clearColor, viewport);
    }

    if (currentScreenTransition)
    {
        currentScreenTransition->SystemDraw(baseGeometricData);
    }
    else if (currentScreen)
    {
        currentScreen->SystemDraw(baseGeometricData);
    }

    popupContainer->SystemDraw(baseGeometricData);

    if (frameSkip > 0)
    {
        frameSkip--;
    }
    //Logger::Info("UIControlSystem::draws: %d", drawCounter);

    FrameOcclusionQueryManager::Instance()->EndQuery(FRAME_QUERY_UI_DRAW);

    GetScreenshoter()->OnFrame();

    TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "UIControlSystem::Draw")
}

void UIControlSystem::SwitchInputToControl(uint32 eventID, UIControl* targetControl)
{
    for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
    {
        if ((*it).touchId == eventID)
        {
            CancelInput(&(*it));

            if (targetControl->IsPointInside((*it).point))
            {
                (*it).controlState = UIEvent::CONTROL_STATE_INSIDE;
                targetControl->touchesInside++;
            }
            else
            {
                (*it).controlState = UIEvent::CONTROL_STATE_OUTSIDE;
            }
            (*it).touchLocker = targetControl;
            targetControl->currentInputID = eventID;
            if (targetControl->GetExclusiveInput())
            {
                SetExclusiveInputLocker(targetControl, eventID);
            }
            else
            {
                SetExclusiveInputLocker(NULL, -1);
            }

            targetControl->totalTouches++;
        }
    }
}

void UIControlSystem::OnInput(UIEvent* newEvent)
{
    inputCounter = 0;

    newEvent->point = VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual(newEvent->physPoint);
    newEvent->tapCount = CalculatedTapCount(newEvent);

    if (Replay::IsPlayback())
    {
        return;
    }

    if (lockInputCounter > 0)
    {
        return;
    }

    if (InputSystem::Instance()->SkipInputEvents(newEvent))
        return;

    if (frameSkip <= 0)
    {
        if (Replay::IsRecord())
        {
            Replay::Instance()->RecordEvent(newEvent);
        }
        UIEvent* eventToHandle = nullptr;

        if (newEvent->phase == UIEvent::Phase::BEGAN || newEvent->phase == UIEvent::Phase::DRAG || newEvent->phase == UIEvent::Phase::ENDED || newEvent->phase == UIEvent::Phase::CANCELLED)
        {
            auto it = std::find_if(begin(touchEvents), end(touchEvents), [newEvent](const UIEvent& ev) {
                return ev.touchId == newEvent->touchId;
            });
            if (it == end(touchEvents))
            {
                touchEvents.push_back(*newEvent);
                eventToHandle = &touchEvents.back();
            }
            else
            {
                it->timestamp = newEvent->timestamp;
                it->physPoint = newEvent->physPoint;
                it->point = newEvent->point;
                it->tapCount = newEvent->tapCount;
                it->phase = newEvent->phase;
                it->inputHandledType = newEvent->inputHandledType;

                eventToHandle = &(*it);
            }
        }
        else
        {
            eventToHandle = newEvent;
        }

        if (currentScreen)
        {
            if (!popupContainer->SystemInput(eventToHandle))
            {
                currentScreen->SystemInput(eventToHandle);
            }
        }

        auto startRemoveIt = std::remove_if(begin(touchEvents), end(touchEvents), [this](UIEvent& ev) {
            bool shouldRemove = (ev.phase == UIEvent::Phase::ENDED || ev.phase == UIEvent::Phase::CANCELLED);
            if (shouldRemove)
            {
                CancelInput(&ev);
            }
            return shouldRemove;
        });
        touchEvents.erase(startRemoveIt, end(touchEvents));
    } // end if frameSkip <= 0
}

void UIControlSystem::CancelInput(UIEvent* touch)
{
    if (touch->touchLocker)
    {
        touch->touchLocker->SystemInputCancelled(touch);
    }
    if (touch->touchLocker != currentScreen.Get())
    {
        currentScreen->SystemInputCancelled(touch);
    }
}
void UIControlSystem::CancelAllInputs()
{
    for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
    {
        CancelInput(&(*it));
    }
    touchEvents.clear();
}

void UIControlSystem::CancelInputs(UIControl* control, bool hierarchical)
{
    for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
    {
        if (!hierarchical)
        {
            if (it->touchLocker == control)
            {
                CancelInput(&(*it));
                break;
            }
            continue;
        }
        UIControl* parentLockerControl = it->touchLocker;
        while (parentLockerControl)
        {
            if (control == parentLockerControl)
            {
                CancelInput(&(*it));
                break;
            }
            parentLockerControl = parentLockerControl->GetParent();
        }
    }
}

int32 UIControlSystem::LockInput()
{
    lockInputCounter++;
    if (lockInputCounter > 0)
    {
        CancelAllInputs();
    }
    return lockInputCounter;
}

int32 UIControlSystem::UnlockInput()
{
    DVASSERT(lockInputCounter != 0);

    lockInputCounter--;
    if (lockInputCounter == 0)
    {
        // VB: Done that because hottych asked to do that.
        CancelAllInputs();
    }
    return lockInputCounter;
}

int32 UIControlSystem::GetLockInputCounter() const
{
    return lockInputCounter;
}

const Vector<UIEvent>& UIControlSystem::GetAllInputs()
{
    return touchEvents;
}

void UIControlSystem::SetExclusiveInputLocker(UIControl* locker, uint32 lockEventId)
{
    SafeRelease(exclusiveInputLocker);
    if (locker != NULL)
    {
        for (Vector<UIEvent>::iterator it = touchEvents.begin(); it != touchEvents.end(); it++)
        {
            if (it->touchId != lockEventId && it->touchLocker != locker)
            { //cancel all inputs excepts current input and inputs what allready handles by this locker.
                CancelInput(&(*it));
            }
        }
    }

    exclusiveInputLocker = SafeRetain(locker);
}

UIControl* UIControlSystem::GetExclusiveInputLocker()
{
    return exclusiveInputLocker;
}

void UIControlSystem::ScreenSizeChanged(const Rect& newFullscreenRect)
{
    if (fullscreenRect == newFullscreenRect)
    {
        return;
    }

    resizePerFrame++;
    if (resizePerFrame >= 5)
    {
        Logger::Error("Resizes per frame : %d", resizePerFrame);
    }

    fullscreenRect = newFullscreenRect;

    if (currentScreenTransition.Valid())
    {
        currentScreenTransition->SystemScreenSizeChanged(fullscreenRect);
    }

    if (currentScreen.Valid())
    {
        currentScreen->SystemScreenSizeChanged(fullscreenRect);
    }

    popupContainer->SystemScreenSizeChanged(fullscreenRect);
}

void UIControlSystem::SetHoveredControl(UIControl* newHovered)
{
    if (hovered != newHovered)
    {
        if (hovered)
        {
            hovered->SystemDidRemoveHovered();
            hovered->Release();
        }
        hovered = SafeRetain(newHovered);
        if (hovered)
        {
            hovered->SystemDidSetHovered();
        }
    }
}

UIControl* UIControlSystem::GetHoveredControl(UIControl* newHovered)
{
    return hovered;
}

void UIControlSystem::SetFocusedControl(UIControl* newFocused, bool forceSet)
{
    if (focusedControl)
    {
        if (forceSet || focusedControl->IsLostFocusAllowed(newFocused))
        {
            focusedControl->SystemOnFocusLost(newFocused);
            SafeRelease(focusedControl);
            focusedControl = SafeRetain(newFocused);
            if (focusedControl)
            {
                focusedControl->SystemOnFocused();
            }
        }
    }
    else
    {
        focusedControl = SafeRetain(newFocused);
        if (focusedControl)
        {
            focusedControl->SystemOnFocused();
        }
    }
}

UIControl* UIControlSystem::GetFocusedControl()
{
    return focusedControl;
}

const UIGeometricData& UIControlSystem::GetBaseGeometricData() const
{
    return baseGeometricData;
}

void UIControlSystem::ReplayEvents()
{
    while (Replay::Instance()->IsEvent())
    {
        int32 activeCount = Replay::Instance()->PlayEventsCount();
        while (activeCount--)
        {
            UIEvent ev = Replay::Instance()->PlayEvent();
            OnInput(&ev);
        }
    }
}

int32 UIControlSystem::LockSwitch()
{
    screenLockCount++;
    return screenLockCount;
}

int32 UIControlSystem::UnlockSwitch()
{
    screenLockCount--;
    DVASSERT(screenLockCount >= 0);
    return screenLockCount;
}

void UIControlSystem::AddScreenSwitchListener(ScreenSwitchListener* listener)
{
    screenSwitchListeners.push_back(listener);
}

void UIControlSystem::RemoveScreenSwitchListener(ScreenSwitchListener* listener)
{
    Vector<ScreenSwitchListener*>::iterator it = std::find(screenSwitchListeners.begin(), screenSwitchListeners.end(), listener);
    if (it != screenSwitchListeners.end())
        screenSwitchListeners.erase(it);
}

void UIControlSystem::NotifyListenersWillSwitch(UIScreen* screen)
{
    // TODO do we need Copy?
    Vector<ScreenSwitchListener*> screenSwitchListenersCopy = screenSwitchListeners;
    for (auto& listener : screenSwitchListenersCopy)
    {
        listener->OnScreenWillSwitch(screen);
    }
}

void UIControlSystem::NotifyListenersDidSwitch(UIScreen* screen)
{
    // TODO do we need Copy?
    Vector<ScreenSwitchListener*> screenSwitchListenersCopy = screenSwitchListeners;
    for (auto& listener : screenSwitchListenersCopy)
    {
        listener->OnScreenDidSwitch(screen);
    }
}

int32 UIControlSystem::CalculatedTapCount(UIEvent* newEvent)
{
    int32 tapCount = 0;
    // Observe double click, doubleClickTime - interval between newEvent and lastEvent, doubleClickRadiusSquared - radius in squared
    if (newEvent->phase == UIEvent::Phase::BEGAN)
    {
        DVASSERT(newEvent->tapCount == 0 && "Native implementation disabled, tapCount must be 0");
        tapCount = 1;
        // only if last event ended
        if (lastClickData.lastClickEnded)
        {
            if ((lastClickData.timestamp != 0.0) && ((newEvent->timestamp - lastClickData.timestamp) < doubleClickTime))
            {
                // if point inside circle = (x0-x1)(x0-x1) + (y0-y1)(y0-y1) < r*r
                float32 pointShift((lastClickData.physPoint.x - newEvent->physPoint.x) * (lastClickData.physPoint.x - newEvent->physPoint.x));
                pointShift += (lastClickData.physPoint.y - newEvent->physPoint.y) * (lastClickData.physPoint.y - newEvent->physPoint.y);
                if (static_cast<int32>(pointShift) < doubleClickRadiusSquared)
                {
                    tapCount = lastClickData.tapCount + 1;
                }
            }
        }
        lastClickData.touchId = newEvent->touchId;
        lastClickData.timestamp = newEvent->timestamp;
        lastClickData.physPoint = newEvent->physPoint;
        lastClickData.tapCount = tapCount;
        lastClickData.lastClickEnded = false;
    }
    else if (newEvent->phase == UIEvent::Phase::ENDED)
    {
        if (newEvent->touchId == lastClickData.touchId)
        {
            lastClickData.lastClickEnded = true;
        }
    }
    return tapCount;
}

bool UIControlSystem::IsRtl() const
{
    return layoutSystem->IsRtl();
}

void UIControlSystem::SetRtl(bool rtl)
{
    layoutSystem->SetRtl(rtl);
}

bool UIControlSystem::IsBiDiSupportEnabled() const
{
    return TextBlock::IsBiDiSupportEnabled();
}

void UIControlSystem::SetBiDiSupportEnabled(bool support)
{
    TextBlock::SetBiDiSupportEnabled(support);
}

bool UIControlSystem::IsHostControl(const UIControl* control) const
{
    return (GetScreen() == control || GetPopupContainer() == control || GetScreenTransition() == control);
}

UILayoutSystem* UIControlSystem::GetLayoutSystem() const
{
    return layoutSystem;
}

UIStyleSheetSystem* UIControlSystem::GetStyleSheetSystem() const
{
    return styleSheetSystem;
}

UIScreenshoter* UIControlSystem::GetScreenshoter()
{
    return screenshoter;
}

void UIControlSystem::SetClearColor(const DAVA::Color& clearColor)
{
    RenderSystem2D::RenderTargetPassDescriptor newDescr = RenderSystem2D::Instance()->GetMainTargetDescriptor();
    newDescr.clearColor = clearColor;
    RenderSystem2D::Instance()->SetMainTargetDescriptor(newDescr);
}

void UIControlSystem::SetUseClearPass(bool useClearPass)
{
    RenderSystem2D::RenderTargetPassDescriptor newDescr = RenderSystem2D::Instance()->GetMainTargetDescriptor();
    newDescr.clearTarget = useClearPass;
    RenderSystem2D::Instance()->SetMainTargetDescriptor(newDescr);
}

void UIControlSystem::SetDefaultTapCountSettings()
{
    doubleClickTime = defaultDoubleClickTime;
    doubleClickRadiusSquared = defaultDoubleClickRadiusSquared;
}

void UIControlSystem::SetTapCountSettings(float32 time, int32 radius)
{
    DVASSERT((time > 0.f) && (radius > 0));
    doubleClickTime = time;
    doubleClickRadiusSquared = radius * radius;
}
};
