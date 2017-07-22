#include "Tests/NotificationTest.h"
#include "Base/Message.h"
#include "UI/Focus/UIFocusComponent.h"
#include "UI/Update/UIUpdateComponent.h"
#include "UI/Render/UIDebugRenderComponent.h"

using namespace DAVA;

NotificationScreen::NotificationScreen(TestBed& app)
    : BaseScreen(app, "NotificationScreen")
    , showNotificationText(nullptr)
    , showNotificationTextDelayed(nullptr)
    , cancelDelayedNotifications(nullptr)
    , showNotificationProgress(nullptr)
    , hideNotificationProgress(nullptr)
    , notificationProgress(nullptr)
    , notificationText(nullptr)
    , progress(0)
{
    GetOrCreateComponent<UIUpdateComponent>();
}

void NotificationScreen::LoadResources()
{
    BaseScreen::LoadResources();
    Font* font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);

    font->SetSize(30);

    showNotificationText = new UIButton(Rect(10, 10, 450, 60));
    showNotificationText->SetStateFont(0xFF, font);
    showNotificationText->SetStateFontColor(0xFF, Color::White);
    showNotificationText->SetStateText(0xFF, L"Notify text");

    showNotificationText->GetOrCreateComponent<UIDebugRenderComponent>();
    showNotificationText->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NotificationScreen::OnNotifyText));
    AddControl(showNotificationText);

    showNotificationTextDelayed = new UIButton(Rect(10, 100, 400, 60));
    showNotificationTextDelayed->SetStateFont(0xFF, font);
    showNotificationTextDelayed->SetStateFontColor(0xFF, Color::White);
    showNotificationTextDelayed->SetStateText(0xFF, L"Notify text after X seconds");

    showNotificationTextDelayed->GetOrCreateComponent<UIDebugRenderComponent>();
    showNotificationTextDelayed->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NotificationScreen::OnNotifyTextDelayed));
    AddControl(showNotificationTextDelayed);

    cancelDelayedNotifications = new UIButton(Rect(10, 200, 450, 60));
    cancelDelayedNotifications->SetStateFont(0xFF, font);
    cancelDelayedNotifications->SetStateFontColor(0xFF, Color::White);
    cancelDelayedNotifications->SetStateText(0xFF, L"Cancel all delayed notifications");

    cancelDelayedNotifications->GetOrCreateComponent<UIDebugRenderComponent>();
    cancelDelayedNotifications->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NotificationScreen::OnNotifyCancelDelayed));
    AddControl(cancelDelayedNotifications);

    hideNotificationText = new UIButton(Rect(10, 300, 450, 60));
    hideNotificationText->SetStateFont(0xFF, font);
    hideNotificationText->SetStateFontColor(0xFF, Color::White);
    hideNotificationText->SetStateText(0xFF, L"Hide text");

    hideNotificationText->GetOrCreateComponent<UIDebugRenderComponent>();
    hideNotificationText->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NotificationScreen::OnHideText));
    AddControl(hideNotificationText);

    showNotificationProgress = new UIButton(Rect(500, 10, 450, 60));
    showNotificationProgress->SetStateFont(0xFF, font);
    showNotificationProgress->SetStateFontColor(0xFF, Color::White);
    showNotificationProgress->SetStateText(0xFF, L"Notify progress");

    showNotificationProgress->GetOrCreateComponent<UIDebugRenderComponent>();
    showNotificationProgress->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NotificationScreen::OnNotifyProgress));
    AddControl(showNotificationProgress);

    hideNotificationProgress = new UIButton(Rect(500, 100, 450, 60));
    hideNotificationProgress->SetStateFont(0xFF, font);
    hideNotificationProgress->SetStateFontColor(0xFF, Color::White);
    hideNotificationProgress->SetStateText(0xFF, L"Hide progress");

    hideNotificationProgress->GetOrCreateComponent<UIDebugRenderComponent>();
    hideNotificationProgress->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NotificationScreen::OnHideProgress));
    AddControl(hideNotificationProgress);

    activateFromNotification = new UIStaticText(Rect(10, 400, 450, 60));
    activateFromNotification->SetTextColor(Color::White);
    activateFromNotification->SetFont(font);
    activateFromNotification->SetMultiline(true);
    activateFromNotification->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(activateFromNotification);

    notificationDelayTextField = new UITextField(Rect(420, 100, 35, 60));
    notificationDelayTextField->GetOrCreateComponent<UIFocusComponent>();
    notificationDelayTextField->SetFont(font);
    notificationDelayTextField->GetOrCreateComponent<UIDebugRenderComponent>();
    notificationDelayTextField->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    notificationDelayTextField->SetText(L"5");
    AddControl(notificationDelayTextField);

    LocalNotificationController::Instance()->RequestPermissions();

    SafeRelease(font);
}

void NotificationScreen::UnloadResources()
{
    BaseScreen::UnloadResources();

    RemoveAllControls();
    SafeRelease(activateFromNotification);
    SafeRelease(showNotificationText);
    SafeRelease(showNotificationProgress);
    SafeRelease(hideNotificationProgress);
    SafeRelease(notificationDelayTextField);
}

void NotificationScreen::Update(float32 timeElapsed)
{
    BaseScreen::Update(timeElapsed);

    if (nullptr == notificationProgress)
        return;

    static float32 timeCounter = 0;
    timeCounter += timeElapsed;

    if (0.25 <= timeCounter)
    {
        timeCounter = 0;

        if (100 == progress)
        {
            progress = 0;
        }

        notificationProgress->SetProgressCurrent(progress++);
    }
}

void NotificationScreen::Draw(const UIGeometricData& geometricData)
{
}

void NotificationScreen::UpdateNotification()
{
    if (nullptr == notificationProgress)
        return;

    if (100 == progress)
    {
        progress = 0;
    }

    notificationProgress->SetProgressCurrent(progress++);
}

void NotificationScreen::OnNotifyText(BaseObject* obj, void* data, void* callerData)
{
    if (nullptr == notificationText)
    {
        notificationText = LocalNotificationController::Instance()->CreateNotificationText();
        notificationText->Update();

        notificationText->SetAction(Message(this, &NotificationScreen::OnNotificationTextPressed));
    }
    else
    {
        notificationText->Show();
    }

    notificationText->SetTitle(L"Application is on foreground!");
    notificationText->SetText(L"This text appeared at button press ");

    hideNotificationText->GetOrCreateComponent<UIDebugRenderComponent>();
}

void NotificationScreen::OnNotifyTextDelayed(BaseObject* obj, void* data, void* callerData)
{
    int delayInSeconds = std::atoi(UTF8Utils::EncodeToUTF8(notificationDelayTextField->GetText()).c_str());
    LocalNotificationController::Instance()->PostDelayedNotification(L"Test Delayed notification Title", L"Some text", delayInSeconds);
}

void NotificationScreen::OnNotifyCancelDelayed(BaseObject* obj, void* data, void* callerData)
{
    LocalNotificationController::Instance()->RemoveAllDelayedNotifications();
}

void NotificationScreen::OnHideText(BaseObject* obj, void* data, void* callerData)
{
    if (notificationText && notificationText->IsVisible())
    {
        notificationText->Hide();
        hideNotificationText->GetOrCreateComponent<UIDebugRenderComponent>()->SetEnabled(false);
    }
    activateFromNotification->SetText(L"");
}

void NotificationScreen::OnNotifyProgress(BaseObject* obj, void* data, void* callerData)
{
    if (nullptr == notificationProgress)
    {
        notificationProgress = LocalNotificationController::Instance()->CreateNotificationProgress(L"", L"", 100, 0);
        notificationProgress->SetAction(Message(this, &NotificationScreen::OnNotificationProgressPressed));
    }
    else
    {
        notificationProgress->Show();
    }

    notificationProgress->SetTitle(L"Fake Download Progress");
    notificationProgress->SetText(L"You pressed the button");

    hideNotificationProgress->GetOrCreateComponent<UIDebugRenderComponent>()->SetEnabled(true);
}

void NotificationScreen::OnHideProgress(BaseObject* obj, void* data, void* callerData)
{
    if (notificationProgress && notificationProgress->IsVisible())
    {
        notificationProgress->Hide();
        hideNotificationProgress->GetOrCreateComponent<UIDebugRenderComponent>()->SetEnabled(false);
    }
}

void NotificationScreen::OnNotificationTextPressed(BaseObject* obj, void* data, void* callerData)
{
    activateFromNotification->SetText(L"Application activate after NotificationTextPressed\n Press Hide text button");
}

void NotificationScreen::OnNotificationProgressPressed(BaseObject* obj, void* data, void* callerData)
{
}
