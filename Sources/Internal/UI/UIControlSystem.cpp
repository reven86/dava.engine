#include "UI/UIControlSystem.h"
#include "UI/UIScreen.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "Logger/Logger.h"
#include "Debug/DVAssert.h"
#include "Time/SystemTimer.h"
#include "Debug/Replay.h"
#include "UI/UIControlSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "UI/UISystem.h"
#include "UI/Layouts/UILayoutSystem.h"
#include "UI/Focus/UIFocusSystem.h"
#include "UI/Input/UIInputSystem.h"
#include "UI/Scroll/UIScrollBarLinkSystem.h"
#include "UI/Scroll/UIScrollSystem.h"
#include "UI/Sound/UISoundSystem.h"
#include "Render/Renderer.h"
#include "Render/RenderHelper.h"
#include "UI/UIScreenshoter.h"
#include "UI/UIScreenTransition.h"
#include "UI/UIEvent.h"
#include "UI/UIPopup.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Render/2D/TextBlock.h"
#include "Platform/DPIHelper.h"
#include "Platform/DeviceInfo.h"
#include "Input/InputSystem.h"
#include "UI/Update/UIUpdateSystem.h"
#include "Debug/ProfilerOverlay.h"
#include "Engine/Engine.h"
#include "Input/MouseDevice.h"

namespace DAVA
{
UIControlSystem::UIControlSystem()
{
    baseGeometricData.position = Vector2(0, 0);
    baseGeometricData.size = Vector2(0, 0);
    baseGeometricData.pivotPoint = Vector2(0, 0);
    baseGeometricData.scale = Vector2(1.0f, 1.0f);
    baseGeometricData.angle = 0;

    AddSystem(std::make_unique<UIInputSystem>());
    AddSystem(std::make_unique<UIUpdateSystem>());
    AddSystem(std::make_unique<UIStyleSheetSystem>());
    AddSystem(std::make_unique<UILayoutSystem>());
    AddSystem(std::make_unique<UIScrollSystem>());
    AddSystem(std::make_unique<UIScrollBarLinkSystem>());
    AddSystem(std::make_unique<UISoundSystem>());

    inputSystem = GetSystem<UIInputSystem>();
    styleSheetSystem = GetSystem<UIStyleSheetSystem>();
    layoutSystem = GetSystem<UILayoutSystem>();
    soundSystem = GetSystem<UISoundSystem>();
    updateSystem = GetSystem<UIUpdateSystem>();

#if defined(__DAVAENGINE_COREV2__)
    vcs = new VirtualCoordinatesSystem();
    vcs->EnableReloadResourceOnResize(true);
#else
    vcs = VirtualCoordinatesSystem::Instance();
#endif
    vcs->virtualSizeChanged.Connect(this, [](const Size2i&) { TextBlock::ScreenResolutionChanged(); });
    vcs->physicalSizeChanged.Connect(this, [](const Size2i&) { TextBlock::ScreenResolutionChanged(); });

    screenshoter = new UIScreenshoter();

    popupContainer.Set(new UIControl(Rect(0, 0, 1, 1)));
    popupContainer->SetName("UIControlSystem_popupContainer");
    popupContainer->SetInputEnabled(false);
    popupContainer->InvokeActive(UIControl::eViewState::VISIBLE);
    inputSystem->SetPopupContainer(popupContainer.Get());
    styleSheetSystem->SetPopupContainer(popupContainer);
    layoutSystem->SetPopupContainer(popupContainer);

#if !defined(__DAVAENGINE_COREV2__)
    // calculate default radius
    if (DeviceInfo::IsHIDConnected(DeviceInfo::eHIDType::HID_TOUCH_TYPE))
    {
        // quarter of an inch
        defaultDoubleClickRadiusSquared = DPIHelper::GetScreenDPI() * 0.25f;
        if (DeviceInfo::GetScreenInfo().scale != 0.f)
        {
            // to look the same on all devices
            defaultDoubleClickRadiusSquared = defaultDoubleClickRadiusSquared / DeviceInfo::GetScreenInfo().scale;
        }
        defaultDoubleClickRadiusSquared *= defaultDoubleClickRadiusSquared;
    }
    else
    {
        defaultDoubleClickRadiusSquared = 4.f; // default, if touch didn't detect, 4 - default pixels in windows desktop
    }
    doubleClickTime = defaultDoubleClickTime;
    doubleClickRadiusSquared = defaultDoubleClickRadiusSquared;
    doubleClickPhysSquare = defaultDoubleClickRadiusSquared;
#else
    SetDoubleTapSettings(0.5f, 0.25f);
#endif

    ui3DViewCount = 0;
}

UIControlSystem::~UIControlSystem()
{
    inputSystem->SetPopupContainer(nullptr);
    inputSystem->SetCurrentScreen(nullptr);
    styleSheetSystem->SetPopupContainer(RefPtr<UIControl>());
    styleSheetSystem->SetCurrentScreen(RefPtr<UIScreen>());
    styleSheetSystem->SetCurrentScreenTransition(RefPtr<UIScreenTransition>());
    layoutSystem->SetPopupContainer(RefPtr<UIControl>());
    layoutSystem->SetCurrentScreen(RefPtr<UIScreen>());
    layoutSystem->SetCurrentScreenTransition(RefPtr<UIScreenTransition>());

    popupContainer->InvokeInactive();
    popupContainer = nullptr;

    if (currentScreen.Valid())
    {
        currentScreen->InvokeInactive();
        currentScreen = nullptr;
    }

    soundSystem = nullptr;
    inputSystem = nullptr;
    styleSheetSystem = nullptr;
    layoutSystem = nullptr;
    updateSystem = nullptr;

    systems.clear();
    SafeDelete(screenshoter);
    SafeDelete(vcs);
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
    inputSystem->SetCurrentScreen(nullptr);
    styleSheetSystem->SetCurrentScreen(RefPtr<UIScreen>());
    layoutSystem->SetCurrentScreen(RefPtr<UIScreen>());
    SetScreen(nullptr);
}

void UIControlSystem::UpdateControl(UIControl* control)
{
    styleSheetSystem->Update(control);
    layoutSystem->Update(control);
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
            inputSystem->SetCurrentScreen(currentScreen.Get());
            styleSheetSystem->SetCurrentScreen(currentScreen);
            layoutSystem->SetCurrentScreen(currentScreen);

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
        inputSystem->SetCurrentScreen(currentScreen.Get());
        styleSheetSystem->SetCurrentScreen(currentScreen);
        layoutSystem->SetCurrentScreen(currentScreen);

        NotifyListenersDidSwitch(currentScreen.Get());

        if (nextScreenTransitionProcessed)
        {
            nextScreenTransitionProcessed->SetDestinationScreen(currentScreen.Get());

            LockSwitch();
            LockInput();

            currentScreenTransition = nextScreenTransitionProcessed;
            currentScreenTransition->InvokeActive(UIControl::eViewState::VISIBLE);
            styleSheetSystem->SetCurrentScreenTransition(currentScreenTransition);
            layoutSystem->SetCurrentScreenTransition(currentScreenTransition);
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
            styleSheetSystem->SetCurrentScreenTransition(currentScreenTransition);
            layoutSystem->SetCurrentScreenTransition(currentScreenTransition);

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
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::UI_UPDATE);

    updateCounter = 0;
    ProcessScreenLogic();

    float32 timeElapsed = SystemTimer::GetFrameDelta();

    for (auto& system : systems)
    {
        system->Process(timeElapsed);
    }

    RenderSystem2D::RenderTargetPassDescriptor newDescr = RenderSystem2D::Instance()->GetMainTargetDescriptor();
    newDescr.clearTarget = (ui3DViewCount == 0 || currentScreenTransition) && needClearMainPass;
    RenderSystem2D::Instance()->SetMainTargetDescriptor(newDescr);

    //Logger::Info("UIControlSystem::updates: %d", updateCounter);
}

void UIControlSystem::Draw()
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::UI_DRAW);

    resizePerFrame = 0;

    drawCounter = 0;

    if (currentScreenTransition)
    {
        currentScreenTransition->SystemDraw(baseGeometricData, nullptr);
    }
    else if (currentScreen)
    {
        currentScreen->SystemDraw(baseGeometricData, nullptr);
    }

    popupContainer->SystemDraw(baseGeometricData, nullptr);

    if (frameSkip > 0)
    {
        frameSkip--;
    }

    GetScreenshoter()->OnFrame();
}

void UIControlSystem::SwitchInputToControl(uint32 eventID, UIControl* targetControl)
{
    return inputSystem->SwitchInputToControl(eventID, targetControl);
}

void UIControlSystem::OnInput(UIEvent* newEvent)
{
    inputCounter = 0;

    newEvent->point = UIControlSystem::Instance()->vcs->ConvertInputToVirtual(newEvent->physPoint);
    newEvent->tapCount = CalculatedTapCount(newEvent);

    if (Replay::IsPlayback())
    {
        return;
    }

    if (lockInputCounter > 0)
    {
        return;
    }

#if !defined(__DAVAENGINE_COREV2__)
    if (InputSystem::Instance()->GetMouseDevice().SkipEvents(newEvent))
        return;
#endif // !defined(__DAVAENGINE_COREV2__)

    if (ProfilerOverlay::globalProfilerOverlay && ProfilerOverlay::globalProfilerOverlay->OnInput(newEvent))
        return;

    if (frameSkip <= 0)
    {
        if (Replay::IsRecord())
        {
            Replay::Instance()->RecordEvent(newEvent);
        }
        inputSystem->HandleEvent(newEvent);
    } // end if frameSkip <= 0
}

void UIControlSystem::CancelInput(UIEvent* touch)
{
    inputSystem->CancelInput(touch);
}

void UIControlSystem::CancelAllInputs()
{
    inputSystem->CancelAllInputs();
}

void UIControlSystem::CancelInputs(UIControl* control, bool hierarchical)
{
    inputSystem->CancelInputs(control, hierarchical);
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

const Vector<UIEvent>& UIControlSystem::GetAllInputs() const
{
    return inputSystem->GetAllInputs();
}

void UIControlSystem::SetExclusiveInputLocker(UIControl* locker, uint32 lockEventId)
{
    inputSystem->SetExclusiveInputLocker(locker, lockEventId);
}

UIControl* UIControlSystem::GetExclusiveInputLocker() const
{
    return inputSystem->GetExclusiveInputLocker();
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
    inputSystem->SetHoveredControl(newHovered);
}

UIControl* UIControlSystem::GetHoveredControl() const
{
    return inputSystem->GetHoveredControl();
}

void UIControlSystem::SetFocusedControl(UIControl* newFocused)
{
    GetFocusSystem()->SetFocusedControl(newFocused);
}

UIControl* UIControlSystem::GetFocusedControl() const
{
    return GetFocusSystem()->GetFocusedControl();
}

void UIControlSystem::ProcessControlEvent(int32 eventType, const UIEvent* uiEvent, UIControl* control)
{
    soundSystem->ProcessControlEvent(eventType, uiEvent, control);
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

bool UIControlSystem::CheckTimeAndPosition(UIEvent* newEvent)
{
    if ((lastClickData.timestamp != 0.0) && ((newEvent->timestamp - lastClickData.timestamp) < doubleClickTime))
    {
        Vector2 point = lastClickData.physPoint - newEvent->physPoint;
        
#if defined(__DAVAENGINE_COREV2__)
        float32 dpi = GetPrimaryWindow()->GetDPI();
        float32 doubleClickPhysSquare = doubleClickInchSquare * (dpi * dpi);
#endif

        if (point.SquareLength() <= doubleClickPhysSquare)
        {
            return true;
        }
    }
    return false;
}

int32 UIControlSystem::CalculatedTapCount(UIEvent* newEvent)
{
    int32 tapCount = 1;

    // Observe double click:
    // doubleClickTime - interval between newEvent and lastEvent,
    // doubleClickPhysSquare - square for double click in physical pixels
    if (newEvent->phase == UIEvent::Phase::BEGAN)
    {
        DVASSERT(newEvent->tapCount == 0 && "Native implementation disabled, tapCount must be 0");
        // only if last event ended
        if (lastClickData.lastClickEnded)
        {
            if (CheckTimeAndPosition(newEvent))
            {
                tapCount = lastClickData.tapCount + 1;
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
            if (lastClickData.tapCount != 1 && CheckTimeAndPosition(newEvent))
            {
                tapCount = lastClickData.tapCount;
            }
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

void UIControlSystem::RegisterControl(UIControl* control)
{
    for (auto& system : systems)
    {
        system->RegisterControl(control);
    }
}

void UIControlSystem::UnregisterControl(UIControl* control)
{
    for (auto& system : systems)
    {
        system->UnregisterControl(control);
    }
}

void UIControlSystem::RegisterVisibleControl(UIControl* control)
{
    for (auto& system : systems)
    {
        system->OnControlVisible(control);
    }
}

void UIControlSystem::UnregisterVisibleControl(UIControl* control)
{
    for (auto& system : systems)
    {
        system->OnControlInvisible(control);
    }
}

void UIControlSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    for (auto& system : systems)
    {
        system->RegisterComponent(control, component);
    }
}

void UIControlSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    for (auto& system : systems)
    {
        system->UnregisterComponent(control, component);
    }
}

void UIControlSystem::AddSystem(std::unique_ptr<UISystem> system, const UISystem* insertBeforeSystem)
{
    if (insertBeforeSystem)
    {
        auto insertIt = std::find_if(systems.begin(), systems.end(),
                                     [insertBeforeSystem](const std::unique_ptr<UISystem>& systemPtr)
                                     {
                                         return systemPtr.get() == insertBeforeSystem;
                                     });
        DVASSERT(insertIt != systems.end());
        systems.insert(insertIt, std::move(system));
    }
    else
    {
        systems.push_back(std::move(system));
    }
}

std::unique_ptr<UISystem> UIControlSystem::RemoveSystem(const UISystem* system)
{
    auto it = std::find_if(systems.begin(), systems.end(),
                           [system](const std::unique_ptr<UISystem>& systemPtr)
                           {
                               return systemPtr.get() == system;
                           });

    if (it != systems.end())
    {
        std::unique_ptr<UISystem> systemPtr(it->release());
        systems.erase(it);
        return systemPtr;
    }

    return nullptr;
}

UILayoutSystem* UIControlSystem::GetLayoutSystem() const
{
    return layoutSystem;
}

UIInputSystem* UIControlSystem::GetInputSystem() const
{
    return inputSystem;
}

UIFocusSystem* UIControlSystem::GetFocusSystem() const
{
    return inputSystem->GetFocusSystem();
}

UISoundSystem* UIControlSystem::GetSoundSystem() const
{
    return soundSystem;
}

UIStyleSheetSystem* UIControlSystem::GetStyleSheetSystem() const
{
    return styleSheetSystem;
}

UIUpdateSystem* UIControlSystem::GetUpdateSystem() const
{
    return updateSystem;
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
    needClearMainPass = useClearPass;
}

void UIControlSystem::SetDoubleTapSettings(float32 time, float32 inch)
{
    DVASSERT((time > 0.0f) && (inch > 0.0f));
    doubleClickTime = time;

#if !defined(__DAVAENGINE_COREV2__)
    // calculate pixels from inch
    float32 dpi = static_cast<float32>(DPIHelper::GetScreenDPI());
    if (DeviceInfo::GetScreenInfo().scale != 0.f)
    {
        // to look the same on all devices
        dpi /= DeviceInfo::GetScreenInfo().scale;
    }
    doubleClickPhysSquare = inch * dpi;
    doubleClickPhysSquare *= doubleClickPhysSquare;
#else
    doubleClickInchSquare = inch * inch;
#endif
}

void UIControlSystem::UI3DViewAdded()
{
    ui3DViewCount++;
}

void UIControlSystem::UI3DViewRemoved()
{
    DVASSERT(ui3DViewCount);
    ui3DViewCount--;
}

int32 UIControlSystem::GetUI3DViewCount()
{
    return ui3DViewCount;
}
};
